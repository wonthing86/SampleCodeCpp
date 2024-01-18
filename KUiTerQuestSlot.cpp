// Fill out your copyright notice in the Description page of Project Settings.


#include "KUiTerQuestSlot.h"

#include "Account/Interface/IKAcuController.h"
#include "Account/Interface/IKAcuModel.h"
#include "Account/Interface/IKAcuView.h"
#include "Account/Mvc/Model/KAcuQuestDef.h"
#include "Account/Mvc/View/KAcuViewAckCallback.h"
#include "Account/Briefing/Quest/KAcuBfQuest.h"

#include "GameDB/Base/Tables/KQuestRow.h"
#include "Account/Mvc/Model/KAcuSawQuestMgr.h"

#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

#include "Holder/KMvcHolder.h"
#include "Holder/KSystemHolder.h"
#include "System/Interface/IKUiManager.h"

#include "GameDB/Helper/KDbhTer.h"
#include "GameDB/Helper/KDbhLocale.h"
#include "GameDB/Base/Tables/KLocaleRow.h"
//#include "GameDB/Base/Tables/KQuestRow.h"
#include "GameDB/Base/Tables/KManor_AreaRow.h"
#include "GameModule/KSaw/Mvc/Model/KQuestTempData.h"
#include "GameModule/KSaw/Mvc/View/KSawViewDef.h"

#include "GameModule/KTer/KTerPlayerController.h"
#include "GameModule/KTer/Interface/IKTerModel.h"
#include "GameModule/KTer/Interface/IKTerModelAgentNpc.h"
#include "GameModule/KTer/Interface/IKTerView.h"
#include "GameModule/KTer/Interface/IKTerViewAgent.h"
#include "GameModule/KTer/Interface/IKTerViewAgentNpc.h"
#include "GameModule/KTer/KTerBuildingInfoComponent.h"
#include "GameModule/KTer/KTerNpcInfoComponent.h"
#include "GameModule/KTer/Data/KTerQuestEventDataAsset.h"
#include "GameModule/KTer/Briefing/KTerBriefingBase.h"
#include "GameModule/KTer/KTerNpc.h"
#include "GameModule/KTer/KTerObserver.h"
#include "GameModule/KTer/UI/KUiTerQuestList.h"
#include "GameModule/KTer/UI/KUiTerHelper.h"

#include "System/UI/Core/KUiBpHelper.h"
#include "UI/Templates/KUiQuestImg.h"


void UKUiTerQuestSlot::OpenList(int32 uniqueId, bool isUpgrade, bool isComplete)
{
	UniqueId = uniqueId;

	SlotBox->ClearChildren();

	UClass* slotClass = SlotClassPtr.LoadSynchronous();

	TSharedPtr<IKTerModel> terModel = FKMvcHolder::Get().GetTerModel();
	if (!terModel.IsValid())
		return;

	TSharedPtr<IKTerModelAgentNpc> terNpcModelAgent = terModel->FindAgentNpc(uniqueId);
	if (!terNpcModelAgent.IsValid())
		return;

	TArray<FKTempQuestData> questList = terNpcModelAgent->GetPriorityQuestList();

	for (auto questData : questList)
	{
		UKUiTerQuestSlotChild * childSlot = CreateWidget<UKUiTerQuestSlotChild>(this, slotClass);
		if (IsValid(childSlot))
		{
			childSlot->SlotClickEvent.BindUObject(this, &UKUiTerQuestSlot::onClickSlot);
			childSlot->SlotPreClickEvent.BindUObject(this, &UKUiTerQuestSlot::onPreClickSlot);

			childSlot->InitSlot(uniqueId, questData.QuestTid, questData.QuestAgentType);

			auto verticalBoxSlot = SlotBox->AddChildToVerticalBox(childSlot);
			verticalBoxSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 4.f));

			SlotArr.Emplace(childSlot);
		}
	}

	if (isUpgrade)
	{
		UClass* upgradeSlotClass = UpgradeSlotClassPtr.LoadSynchronous();

		UKUiTerUpgradeSlotChild * childSlot = CreateWidget<UKUiTerUpgradeSlotChild>(this, upgradeSlotClass);
		if (IsValid(childSlot))
		{
			childSlot->SlotClickEvent.BindUObject(this, &UKUiTerQuestSlot::onClickUpgradeSlot);
			childSlot->SlotPreClickEvent.BindUObject(this, &UKUiTerQuestSlot::onPreClickUpgradeSlot);

			childSlot->InitSlot(uniqueId, isComplete);

			auto verticalBoxSlot = SlotBox->AddChildToVerticalBox(childSlot);
			verticalBoxSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 4.f));

			UpgradeSlot = childSlot;
		}
	}

	playShowAnim();
}

UUserWidget* UKUiTerQuestSlot::GetQuestSlot(int32 questId)
{
	for (UKUiTerQuestSlotChild* slot : SlotArr)
	{
		if (slot->QuestTid == questId)
		{
			return Cast<UUserWidget>(slot);
		}
	}
	return nullptr;
}

void UKUiTerQuestSlot::onPreClickSlot(UKUiTerQuestSlotChild* clickedSlot)
{
	QuestSlotCallback.ExecuteIfBound();

	for (auto slot : SlotArr)
	{
		if (IsValid(slot) && slot != clickedSlot)
		{
			slot->PlayCollapseAnim();
		}
	}

	if (UpgradeSlot)
		UpgradeSlot->PlayCollapseAnim();
}

void UKUiTerQuestSlot::onPreClickUpgradeSlot(class UKUiTerUpgradeSlotChild* clickedSlot)
{
	QuestSlotCallback.ExecuteIfBound();

	for (auto slot : SlotArr)
	{
		if (IsValid(slot))
		{
			slot->PlayCollapseAnim();
		}
	}
}

void UKUiTerQuestSlot::onClickSlot(class UKUiTerQuestSlotChild* clickedSlot)
{
	if (IsValid(clickedSlot))
	{
		TSharedPtr<IKTerView> terView = FKMvcHolder::Get().GetTerView();
		if (!terView.IsValid())
			return;

		TSharedPtr<IKTerViewAgentNpc> viewAgent = terView->FindViewAgentNpc(UniqueId);
		if (!viewAgent.IsValid())
			return;

		TWeakObjectPtr<UKTerNpcInfoComponent> npcInfoComp = viewAgent->GetAgentComponent();
		if (!npcInfoComp.IsValid())
			return;

		if (clickedSlot->QuestAgentType == E_KQuestAgentType::Giver)
		{
			npcInfoComp->PlayEventBeforeAccept(clickedSlot->QuestTid);
		}
		else if (clickedSlot->QuestAgentType == E_KQuestAgentType::Condition)
		{
			npcInfoComp->ReqQuestNpcTalk(clickedSlot->QuestTid);
		}
		else if (clickedSlot->QuestAgentType == E_KQuestAgentType::Ender)
		{
			npcInfoComp->ReqQuestComplete(clickedSlot->QuestTid);
		}

		auto uiManager = FKSystemHolder::Get().GetUiManager();
		if (uiManager.IsValid())
		{
			const auto& questList = Cast<UKUITerQuestList>(uiManager->FindExistMenu("Manor/Dev/UI/WB_Ter_QuestList"));
			if (IsValid(questList))
			{
				questList->Close();
				FKUiTerHelper::HiddenManorAreaOutline();
			}
		}
	}
}

void UKUiTerQuestSlot::onClickUpgradeSlot(class UKUiTerUpgradeSlotChild* clickedSlot)
{
	if (IsValid(clickedSlot))
	{
		TSharedPtr<IKTerView> terView = FKMvcHolder::Get().GetTerView();
		if (!terView.IsValid())
			return;

		TSharedPtr<IKTerViewAgentNpc> viewAgent = terView->FindViewAgentNpc(UniqueId);
		if (!viewAgent.IsValid())
			return;

		TWeakObjectPtr<UKTerNpcInfoComponent> npcInfoComp = viewAgent->GetAgentComponent();
		if (!npcInfoComp.IsValid())
			return;

		npcInfoComp->PlayEventUpgrade(clickedSlot->LinkedAreaId);

		auto uiManager = FKSystemHolder::Get().GetUiManager();
		if (uiManager.IsValid())
		{
			const auto& questList = Cast<UKUITerQuestList>(uiManager->FindExistMenu("Manor/Dev/UI/WB_Ter_QuestList"));
			if (IsValid(questList))
			{
				questList->Close();
				FKUiTerHelper::HiddenManorAreaOutline();
			}
		}
	}
}

void UKUiTerQuestSlotChild::InitSlot(int32 uniqueId, int32 questTid, E_KQuestAgentType questAgentType)
{
	UniqueId = uniqueId;
	QuestTid = questTid;
	QuestAgentType = questAgentType;

	const FKQuestRow* towQuest = FKQuestRow::Get(questTid);
	if (towQuest)
	{
		FString questNameText = UKDbhLocale::GetText(towQuest->Name_Locale);

		setData_Bp(questNameText);

		// 퀘스트 타입에 따른 아이콘 표기방식 변화로 인해 코드를 추가했습니다. [11/10/2020 jyjung]
		QuestImg->SetQuestIcon(questAgentType, questTid);
	}
}

void UKUiTerQuestSlotChild::OnSlotClicked()
{
	SlotClickEvent.ExecuteIfBound(this);
}

void UKUiTerQuestSlotChild::onSlotPreclicked()
{
	SlotPreClickEvent.ExecuteIfBound(this);
}

void UKUiTerUpgradeSlotChild::InitSlot(int32 uniqueId, bool isComplete)
{
	UniqueId = uniqueId;

	const FKManor_AreaRow * areaRow = UKDbhTer::GetAreaInfoByLinkedNpcId(UniqueId);
	if (areaRow)
	{
		LinkedAreaId = areaRow->Id;

		FString strName = UKDbhLocale::GetText(*areaRow->Name);

		FText localeText;
		const FString localeRow = UKDbhLocale::GetText("UI_ManorAreaUpg_NPC"); /*{ManorAreaType} 업그레이드*/
		localeText = FText::FromString(localeRow);
		localeText = FText::FormatNamed(localeText, TEXT("ManorAreaType"), FText::FromString(strName));

		setData_Bp(localeText.ToString(), isComplete);
	}
}

void UKUiTerUpgradeSlotChild::onSlotClicked()
{
	SlotClickEvent.ExecuteIfBound(this);
}

void UKUiTerUpgradeSlotChild::onSlotPreclicked()
{
	SlotPreClickEvent.ExecuteIfBound(this);
}
