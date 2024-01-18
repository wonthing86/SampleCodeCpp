// Fill out your copyright notice in the Description page of Project Settings.


#include "KTerNpcInfoComponent.h"

#include "Account/Interface/IKAcuController.h"
#include "Account/Interface/IKAcuModel.h"
#include "Account/Interface/IKAcuView.h"
#include "Account/Mvc/Model/KAcuQuestDef.h"
#include "Account/Briefing/Quest/KAcuBfQuest.h"

#include "Account/Mvc/Model/KAcuSawQuestMgr.h"

#include "GameModule/KTer/KTerPlayerController.h"
#include "GameModule/KTer/Interface/IKTerModel.h"
#include "GameModule/KTer/Interface/IKTerModelAgent.h"
#include "GameModule/KTer/Interface/IKTerModelAgentNpc.h"
#include "GameModule/KTer/Interface/IKTerView.h"
#include "GameModule/KTer/Data/KTerQuestEventDataAsset.h"
#include "GameModule/KTer/UI/KUiTerHud.h"
#include "GameModule/KTer/UI/KUiTerHuiNpc.h"
#include "GameModule/KTer/KTerNpc.h"
#include "GameModule/KTer/KTerObserver.h"
#include "GameModule/KTer/UI/KUITerRegionOpenPopup.h"
#include "GameModule/KTer/UI/KUiTerBuildMenuPage.h"
#include "GameModule/KTer/KTerDef.h"

#include "UI/Acu/Chronicle/Tooltip/KUiChroniclesQuestToolTipBox.h"
#include "UI/Templates/KUiPopupToolTipBasic.h"
#include "UI/Commons/Tsw/KUiTswIconHelper.h"

#include "GameDB/Helper/KDbhTer.h"
#include "GameDB/Helper/KDbhLocale.h"
#include "GameDB/Base/Tables/KLocaleRow.h"
#include "GameDB/Base/Tables/KQuestRow.h"
#include "GameDB/Base/Tables/KBubbleTalkRow.h"
#include "GameDB/Base/Tables/KManor_AreaRow.h"

#include "Holder/KMvcHolder.h"
#include "Holder/KSystemHolder.h"
#include "System/Interface/IKContentsLoader.h"
#include "System/Interface/IKUiManager.h"

#include "System/UI/Core/KUiBpHelper.h"
#include "Core/KMacros.h"
#include "Core/KTemplateUtil.h"


// Sets default values for this component's properties
UKTerNpcInfoComponent::UKTerNpcInfoComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UKTerNpcInfoComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}


// Called every frame
void UKTerNpcInfoComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UKTerNpcInfoComponent::initHeadUp()
{
	TSharedPtr<IKTerView> viewSp = FKMvcHolder::Get().GetTerView();
	if (!viewSp.IsValid())
		return;

	TWeakObjectPtr<UKUiTerHud> terHud = viewSp->GetHud();
	if (!terHud.IsValid())
		return;

	TSharedPtr<IKContentsLoader> contentsLoader = FKSystemHolder::Get().GetContentsLoader();
	if (!contentsLoader.IsValid())
		return;

	const FString asssetPath = contentsLoader->GetAssetPath(E_KAssetPath::ArtUiRoot, TEXT("Manor/Dev/UI/WB_Ter_HUI_Npc"));
	UClass* menuClass = contentsLoader->LoadSyncClass(asssetPath);
	if (menuClass)
	{
		HeadUpWidget = CreateWidget<UKUITerHuiNpc>(GetWorld(), menuClass);
		if (HeadUpWidget.IsValid())
		{
			HeadUpWidget->SetUniqueId(NpcUniqueId);
			HeadUpWidget->SetNpcName(NpcName);
			HeadUpWidget->SetBubbleTalkGroupId(BubbleTalkGroupId);

			if (BubbleTalkGroupId > 0)
			{
				HeadUpWidget->SetTalk();
			}

			terHud->AddChildWidgetObjectInfo(HeadUpWidget.Get());
		}
	}
}

void UKTerNpcInfoComponent::resetCamera(bool isHudVisible /*= true*/)
{
	TWeakObjectPtr<AKTerPlayerController> terPC = Cast<AKTerPlayerController>(GetWorld()->GetFirstPlayerController());
	if (terPC.IsValid())
	{
		terPC->SetEnableInput(true);
	}

	if (isHudVisible)
	{
		UKUiBpHelper::SetHudVisibility(GetWorld(), true);
	}
}

void UKTerNpcInfoComponent::onQuestAcceptPanelClicked(E_KUiFrameBtnResult value)
{
	// ok
	if (value == E_KUiFrameBtnResult::Confirm)
	{
		reqQuestAccept(QuestAcceptTid, NpcUniqueId);
		QuestAcceptTid = 0;
	}
	// cancel
	else if (value == E_KUiFrameBtnResult::Cancel)
	{
		/** 다음 액션없이 닫힐 때 Outline 제거*/
		hiddenAreaOutline();
	}
}

void UKTerNpcInfoComponent::reqQuestAccept(int32 questTid, int32 uniqueId)
{
	KACU_REQ_CALL_AND_ACKED_BIND_UOBJECT_INPUT_BLOCK(ReqC2SQuestAccept, this, &UKTerNpcInfoComponent::ackQuestAccept, questTid, uniqueId);
}

E_KAcuBfReply UKTerNpcInfoComponent::ackQuestAccept(TSharedPtr<FKAcuBriefingBase> bfBase)
{
	if (!bfBase.IsValid())
		return E_KAcuBfReply::Handled;

	TSharedPtr<FKAcuBfQuestAccept> bf = KBrifingCast<FKAcuBfQuestAccept>(bfBase);
	if (bf && !bf->IsError())
	{
		TSharedPtr<IKTerModel> terModel = FKMvcHolder::Get().GetTerModel();
		if (terModel.IsValid())
		{
			TSharedPtr<IKTerView> terView = FKMvcHolder::Get().GetTerView();
			if (terView.IsValid())
			{
				PlayEventAfterAccept(bf->GetQuestTid());
			}
		}
	}

	return E_KAcuBfReply::Handled;
}

void UKTerNpcInfoComponent::reqQuestComplete(int32 questTid)
{
	const FKAcuSawQuestMgr& questMgr = FKMvcHolder::Get().GetAccountGetter()->GetSawQuestMgr();
	int64 questId = questMgr.GetQuestDid(questTid);

	KACU_REQ_CALL_AND_ACKED_BIND_UOBJECT_INPUT_BLOCK(ReqC2SQuestComplete, this, &UKTerNpcInfoComponent::ackQuestComplete, questId);
}

E_KAcuBfReply UKTerNpcInfoComponent::ackQuestComplete(TSharedPtr<FKAcuBriefingBase> bfBase)
{
	if (!bfBase.IsValid())
		return E_KAcuBfReply::Handled;

	TSharedPtr<FKAcuBfQuestComplete> bf = KBrifingCast<FKAcuBfQuestComplete>(bfBase);
	if (bf && !bf->IsError())
	{
		checkBuildingUnlock(bf->GetQuestTid());

		PlayEventComplete(bf->GetQuestTid());
	}

	return E_KAcuBfReply::Handled;
}

void UKTerNpcInfoComponent::reqQuestNpcTalk(int32 questTid)
{
	const FKAcuSawQuestMgr& questMgr = FKMvcHolder::Get().GetAccountGetter()->GetSawQuestMgr();

	// 몇번 조건인지 알아야함
	int64 relatedQuestDid = questMgr.GetQuestDid(questTid);
	int32 condNum = 0;
	const FKQuestRow* questRow = FKQuestRow::Get(questTid);
	if (questRow)
	{
		if (questRow->Condition1_Value == NpcUniqueId)
		{
			condNum = 0;
		}
		else if (questRow->Condition2_Value == NpcUniqueId)
		{
			condNum = 1;
		}
		else if (questRow->Condition3_Value == NpcUniqueId)
		{
			condNum = 2;
		}
	}

	KACU_REQ_CALL_AND_ACKED_BIND_UOBJECT_INPUT_BLOCK(ReqC2STownQuestNpcTalk, this, &UKTerNpcInfoComponent::ackQuestNpcTalk, relatedQuestDid, condNum);
}

E_KAcuBfReply UKTerNpcInfoComponent::ackQuestNpcTalk(TSharedPtr<FKAcuBriefingBase> bfBase)
{
	if (!bfBase.IsValid())
		return E_KAcuBfReply::Handled;

	TSharedPtr<FKAcuBfTowQuestNpcTalk> bf = KBrifingCast<FKAcuBfTowQuestNpcTalk>(bfBase);
	if (bf && !bf->IsError())
	{
		PlayEventProgress(bf->GetQuestTid());
	}
	return E_KAcuBfReply::Handled;
}

void UKTerNpcInfoComponent::checkBuildingUnlock(int32 questTid)
{
	const TMap<int32, TUniquePtr<FKManor_AreaRow>>& map = UKDbhTer::GetManorAreaMap();
	for (auto& data : map)
	{
		if (data.Value->QuestId == questTid)
		{
			TSharedPtr<IKTerModel> terModel = FKMvcHolder::Get().GetTerModel();
			if (terModel.IsValid())
			{
				terModel->ForceUpdateAgentUnlockable(data.Value->Id);
			}
			//return;
		}
	}
}

void UKTerNpcInfoComponent::doQuestAutoBeforeAcceptPost()
{
	PlayEventBeforeAccept(QuestAutoTid);
	QuestAutoTid = 0;
}

TWeakObjectPtr<AKTerObserver> UKTerNpcInfoComponent::getObserver()
{
	if (!Observer.IsValid())
	{
		TSharedPtr<IKTerView> viewSp = FKMvcHolder::Get().GetTerView();
		if (viewSp.IsValid())
		{
			Observer = viewSp->GetTerObserver();
		}
	}
	return Observer;
}

void UKTerNpcInfoComponent::Init(TSharedPtr<const IKTerModelAgentNpc>& data)
{
	if (!data.IsValid())
		return;

	NpcUniqueId = data->GetUniqueId();
	NpcName = data->GetName();
	BubbleTalkGroupId = data->GetBubbleTalkGroupId();

	initHeadUp();
}

void UKTerNpcInfoComponent::UpdateLocationHeadUp(const FVector& location)
{
	// 위젯 visiblity 도 같이 업데이트 되므로 visible 이 아니라면 업데이트하지 않는다.
	if (HeadUpWidget.IsValid() && bIsVisible)
		HeadUpWidget->UpdateLocation(location);

	/*if (getObserver().IsValid())
	{
		float zoomRate = FMath::Max(0.8f, 1.0f - getObserver()->GetRelativeZoomRate());
		HeadUpWidget->SetRenderScale(FVector2D(zoomRate, zoomRate));
	}*/
}

void UKTerNpcInfoComponent::PlayIdleTalk(int32 talkIndex)
{
	if (HeadUpQuestType == E_KQuestAgentType::Giver)
		return;

	// #swshin : Npc의 Gesture와 동기화 하기위해 Index로 특정 TalkContents 실행
	if (HeadUpWidget.IsValid())
		HeadUpWidget->PlayIdleTalk(talkIndex);
}

void UKTerNpcInfoComponent::SetSpecialTalk(bool isOn)
{
	if (HeadUpWidget.IsValid())
		HeadUpWidget->SetSpecialTalk(isOn);
}

void UKTerNpcInfoComponent::AddQuestEnderTalk(int32 questTid, const FString& talk)
{
	if (HeadUpWidget.IsValid())
		HeadUpWidget->AddQuestEnderTalk(questTid, talk);
}

void UKTerNpcInfoComponent::RemoveQuestEnderTalk(int32 questTid)
{
	if (HeadUpWidget.IsValid())
		HeadUpWidget->RemoveQuestEnderTalk(questTid);
}

void UKTerNpcInfoComponent::AddTalkConditionId(E_KTalkConditionType conditionType, int32 conditionId)
{
	if (HeadUpWidget.IsValid())
		HeadUpWidget->AddTalkConditionId(conditionType, conditionId);
}

void UKTerNpcInfoComponent::RemoveTalkConditionId(E_KTalkConditionType conditionType, int32 conditionId)
{
	if (HeadUpWidget.IsValid())
		HeadUpWidget->RemoveTalkConditionId(conditionType, conditionId);
}

void UKTerNpcInfoComponent::SetQuest(E_KQuestAgentType nowQuestAgentType, E_KQuestCategory nowQuestCategory)
{
	HeadUpQuestType = nowQuestAgentType;

	if (HeadUpWidget.IsValid())
		HeadUpWidget->SetQuest(nowQuestAgentType, nowQuestCategory);
}

void UKTerNpcInfoComponent::SetUpgrade(bool isUpgrade, int64 completeTime)
{
	if (HeadUpWidget.IsValid())
		HeadUpWidget->SetUpgrade(isUpgrade, completeTime);
}

void UKTerNpcInfoComponent::OpenQuestList()
{
	if (HeadUpWidget.IsValid())
		HeadUpWidget->SetQuestList();
}

void UKTerNpcInfoComponent::OpenQuestAcceptPanel(int32 questTid)
{
	QuestAcceptTid = questTid;

	if (!QuestAcceptPopupDelegate.IsBoundToObject(this))
	{
		QuestAcceptPopupDelegate.BindUObject(this, &UKTerNpcInfoComponent::onQuestAcceptPanelClicked);
	}

	FName titleTextLocale = TEXT("");
	FName categoryLocale = FKUiTswIconHelper::GetQuestCategoryLocale(questTid);

	const FKQuestRow* questRow = FKQuestRow::Get(questTid);
	if (questRow)
	{
		titleTextLocale = questRow->Name_Locale;
	}

	UKUiChroniclesQuestToolTipBox::ShowQuestTooltip(questTid, categoryLocale, titleTextLocale, E_KTooltipBasicBtnType::CancleConfirm, QuestAcceptPopupDelegate);
}

void UKTerNpcInfoComponent::PlayEventBeforeAccept(int32 questTid)
{
	TSharedPtr<IKTerModel> terModel = FKMvcHolder::Get().GetTerModel();
	if (!terModel.IsValid())
		return;

	TSharedPtr<IKTerView> terView = FKMvcHolder::Get().GetTerView();
	if (!terView.IsValid())
		return;

	FKTerQuestEventData * data = terModel->GetQuestEventData(questTid);
	if (data)
	{
		FName eventTag = data->BeforeAcceptEventTag;
		int32 talkTid = data->BeforeAcceptTalk;

		if (!eventTag.IsNone())
		{
			terView->PlayEvent(NpcUniqueId, eventTag, talkTid, questTid);
			return;
		}
	}

	// 연출이 없거나 데이터가 없다?
	OpenQuestAcceptPanel(questTid);
	resetCamera();

	/** 연출이 없으면 Area 아웃라인 바로 제거 */
	hiddenAreaOutline();

	terView->ResetNpcnCamera(NpcUniqueId);
}

void UKTerNpcInfoComponent::PlayEventAfterAccept(int32 questTid)
{
	TSharedPtr<IKTerModel> terModel = FKMvcHolder::Get().GetTerModel();
	if (!terModel.IsValid())
		return;

	TSharedPtr<IKTerView> terView = FKMvcHolder::Get().GetTerView();
	if (!terView.IsValid())
		return;

	FKTerQuestEventData * data = terModel->GetQuestEventData(questTid);
	if (data)
	{
		FName eventTag = data->AfterAcceptEventTag;
		int32 talkTid = data->AfterAcceptTalk;

		if (!eventTag.IsNone())
		{
			terView->PlayEvent(NpcUniqueId, eventTag, talkTid, questTid);
			return;
		}
	}

	// 연출이 없거나 데이터가 없다?
	UKUiBpHelper::ShowQuestAcceptText(GetWorld(), questTid);
	resetCamera();

	/** 연출이 없으면 Area 아웃라인 바로 제거 */
	hiddenAreaOutline();

	terView->ResetNpcnCamera(NpcUniqueId);

	/** 영지 퀘스트 개선 - 컨디션 npc 바로 사라지지 않도록 개선 [1/27/2022 baekseungwon] */
	//terModel->DespawnConditionAgentNpcByCId(questTid, E_KTerNpcDespawnConditionType::AfterAccept);
}

void UKTerNpcInfoComponent::PlayEventComplete(int32 questTid)
{
	TSharedPtr<IKTerModel> terModel = FKMvcHolder::Get().GetTerModel();
	if (!terModel.IsValid())
		return;

	TSharedPtr<IKTerView> terView = FKMvcHolder::Get().GetTerView();
	if (!terView.IsValid())
		return;

	FKTerQuestEventData * data = terModel->GetQuestEventData(questTid);
	if (data)
	{
		FName eventTag = data->CompleteEventTag;
		int32 talkTid = data->CompleteTalk;
		bool bshowResult = data->bShowResult;

		if (!eventTag.IsNone())
		{
			terView->PlayEvent(NpcUniqueId, eventTag, talkTid, questTid, 0, bshowResult);
			return;
		}
	}

	// 연출이 없거나 데이터가 없다?
	FKMvcHolder::Get().GetAccountView()->ShowQuestResultPanel(questTid, true);
	resetCamera();

	/** 연출이 없으면 Area 아웃라인 바로 제거 */
	hiddenAreaOutline();

	terView->ResetNpcnCamera(NpcUniqueId);
	terModel->DespawnConditionAgentNpcByCId(questTid, E_KTerNpcDespawnConditionType::Complete);
}

void UKTerNpcInfoComponent::PlayEventProgress(int32 questTid)
{
	TSharedPtr<IKTerModel> terModel = FKMvcHolder::Get().GetTerModel();
	if (!terModel.IsValid())
		return;

	TSharedPtr<IKTerView> terView = FKMvcHolder::Get().GetTerView();
	if (!terView.IsValid())
		return;

	FKTerQuestEventData * data = terModel->GetQuestEventData(questTid);
	if (data)
	{
		FKTerQuestProgressEventData * progressData = data->ProgressEventDataMap.Find(NpcUniqueId);
		if (progressData)
		{
			FName eventTag = progressData->InteractionEventTag;
			int32 talkTid = progressData->InteractionTalk;

			if (!eventTag.IsNone())
			{
				terView->PlayEvent(NpcUniqueId, eventTag, talkTid, questTid);
				return;
			}
		}
	}

	// 연출이 없거나 데이터가 없다?
	if (!terView->PlayAutoEvent())
	{
		resetCamera();
	}
}

void UKTerNpcInfoComponent::PlayEventUpgrade(int32 areaId)
{
	TSharedPtr<IKTerView> terView = FKMvcHolder::Get().GetTerView();
	if (!terView.IsValid())
		return;

	/*FName eventTag = "ES_Upgrade_Talk";
	int32 talkTid = 1102101;

	if (!eventTag.IsNone())
	{
		terView->PlayEvent(NpcUniqueId, eventTag, talkTid, 0, areaId);
		return;
	}*/

	// 연출이 없거나 데이터가 없다?
	TWeakObjectPtr<UKUiTerHud> terHud = terView->GetHud();
	if (terHud.IsValid())
	{
		bool bisBuildMenuOpen = terHud->IsBuildMenuOpeningUpgrade();
// 		//건설메뉴 확인해서 좌측 입력 여부 전달
// 		const TSharedPtr<IKUiManager> uiManager = FKSystemHolder::Get().GetUiManager();
// 		if (uiManager.IsValid())
// 		{
// 			UKUiTerBuildMenuPage* buildMenu = Cast<UKUiTerBuildMenuPage>(uiManager->FindExistMenu(TEXT("Manor/Dev/UI/WB_Ter_BuildMenu_Page")));

// 			if(IsValid(buildMenu))
// 				bisBuildMenuOpen = true;
// 		}

		terHud->OpenUpgradeMenu(areaId, bisBuildMenuOpen);
		terHud->SetBuildMenuOpeningUpgrade(false);
	}

	resetCamera(false);

	//terView->ResetNpcnCamera(NpcUniqueId);

	if (getObserver().IsValid())
	{
		/** 영지 NPC 상호작용 개선 - 건물이 아닌 NPC 포커싱으로 변경 [1/28/2022 baekseungwon] */
		//getObserver()->SetDistrictFocus(areaId, -0.25f, getObserver()->GetLimitFocusDistance());
		getObserver()->SetNpcFocus(NpcUniqueId, Observer->GetMinDistance(), -0.25f);
	}
}

void UKTerNpcInfoComponent::DoQuestBeforeAutoAccept(int32 questTid)
{
	if (!getObserver().IsValid())
		return;

	if (!getObserver()->PostFocusCallback.IsBound())
	{
		QuestAutoTid = questTid;
		getObserver()->PostFocusCallback.BindUObject(this, &UKTerNpcInfoComponent::doQuestAutoBeforeAcceptPost);
		getObserver()->SetNpcFocus(NpcUniqueId);
	}

	TSharedPtr<IKTerView> viewSp = FKMvcHolder::Get().GetTerView();
	if (viewSp.IsValid())
	{
		TWeakObjectPtr<AKTerNpc> ternpcWp = viewSp->FindSpawnedNpcActor(NpcUniqueId);
		if (ternpcWp.IsValid())
		{
			ternpcWp->SetProgressQuestStatus(true);
		}
	}
}

void UKTerNpcInfoComponent::DoQuestAutoAccept(int32 questTid)
{
	reqQuestAccept(questTid, NpcUniqueId);
}

void UKTerNpcInfoComponent::DoQuestAutoComplete(int32 questTid)
{
	reqQuestComplete(questTid);
}

void UKTerNpcInfoComponent::ReqQuestComplete(int32 questTid)
{
	reqQuestComplete(questTid);
}

void UKTerNpcInfoComponent::ReqQuestNpcTalk(int32 questTid)
{
	reqQuestNpcTalk(questTid);
}

void UKTerNpcInfoComponent::RemoveWidget()
{
	if (HeadUpWidget.IsValid())
	{
		HeadUpWidget->RemoveFromParent();
	}
}

void UKTerNpcInfoComponent::SetFocus()
{
	if (getObserver().IsValid())
	{
		getObserver()->SetNpcFocus(NpcUniqueId, getObserver()->GetLimitFocusDistance());
	}
}

void UKTerNpcInfoComponent::SetHeadUpVisibility(bool bisVisible)
{
	if (HeadUpWidget.IsValid())
	{
		bIsVisible = bisVisible;

		if (bisVisible)
		{
			HeadUpWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else
		{
			HeadUpWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

bool UKTerNpcInfoComponent::IsIdleTalkState()
{
	if (!HeadUpWidget.IsValid())
		return false;

	return HeadUpWidget->IsIdleTalkState();
}

void UKTerNpcInfoComponent::hiddenAreaOutline()
{
	if (getObserver().IsValid())
	{
		getObserver()->HiddenAreaOutLine();
	}
}
