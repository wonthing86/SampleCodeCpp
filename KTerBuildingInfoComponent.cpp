// Fill out your copyright notice in the Description page of Project Settings.


#include "KTerBuildingInfoComponent.h"

#include "GameModule/KTer/Interface/IKTerView.h"
#include "GameModule/KTer/Interface/IKTerModel.h"
#include "GameModule/KTer/Mvc/Model/KTerModelAgent.h"
#include "GameModule/KTer/UI/KUITerBuildingInfo.h"
#include "GameModule/KTer/UI/KUiTerHud.h"
#include "GameModule/KTer/UI/KUITerBuildingUpgradeCompleteBox.h"
#include "GameModule/KTer/KTerBuildingInfo.h"
#include "GameModule/KTer/KTerDef.h"
#include "GameModule/KTer/KTerObserver.h"
#include "GameModule/KTer/KTerPlayerController.h"

#include "UI/Hud/KUiUnLockResult.h"

#include "Holder/KMvcHolder.h"
#include "Holder/KSystemHolder.h"
#include "System/Interface/IKContentsLoader.h"
#include "System/Interface/IKUiManager.h"
#include "System/UI/Core/KUiBpHelper.h"

#include "Components/CanvasPanelSlot.h"

#include "Core/KCommonUtil.h"
#include "Core/KLog.h"

// Sets default values for this component's properties
UKTerBuildingInfoComponent::UKTerBuildingInfoComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
	DefaultMenuButtons.Empty();
}


// Called when the game starts
void UKTerBuildingInfoComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	TerBuildingInfoWp = Cast<AKTerBuildingInfo>(GetOwner());
}


// Called every frame
void UKTerBuildingInfoComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UKTerBuildingInfoComponent::initWidgetBuildingInfo()
{
	if (IsValid(WidgetBuildingInfo))
	{
		updateWidgetBuildingInfo();
		return;
	}

	TSharedPtr<IKTerModel> modelSp = FKMvcHolder::Get().GetTerModel();
	if (!modelSp.IsValid())
		return;

	TSharedPtr<IKTerModelAgent> modelAgentSp = modelSp->FindAgent(UniqueId);
	if (!modelAgentSp.IsValid())
		return;

	TSharedPtr<IKTerView> viewSp = FKMvcHolder::Get().GetTerView();
	if (!viewSp.IsValid())
		return;

	TWeakObjectPtr<UKUiTerHud> terHud = viewSp->GetHud();
	if (!terHud.IsValid())
		return;
	
	TSharedPtr<IKContentsLoader> contentsLoader = FKSystemHolder::Get().GetContentsLoader();
	if (contentsLoader.IsValid())
	{
		const FString asssetPath = contentsLoader->GetAssetPath(E_KAssetPath::ArtUiRoot, TEXT("Hud/Manor/WB_Manor_HUI_Slot"));
		UClass* menuClass = contentsLoader->LoadSyncClass(asssetPath);
		if (menuClass)
		{
			WidgetBuildingInfo = CreateWidget<UKUITerBuildingInfo>(GetWorld(), menuClass);
			if (IsValid(WidgetBuildingInfo))
			{
				terHud->AddChildWidgetBuildingInfo(WidgetBuildingInfo);
				UCanvasPanelSlot* canvasPanelSlot = Cast<UCanvasPanelSlot>(WidgetBuildingInfo->Slot);
				if (IsValid(canvasPanelSlot))
				{
					canvasPanelSlot->SetAlignment(FVector2D(0.5f, 0.0f));
					canvasPanelSlot->SetAutoSize(true);
				}

				E_KTerBuildingType buildingType = GetType();
				WidgetBuildingInfo->SetBuildingIcon(UniqueId, buildingType, Status, RepairType, Level, CompleteTime, ProductTime);
			}
		}
	}
}

void UKTerBuildingInfoComponent::updateWidgetBuildingInfo()
{
	if (!IsValid(WidgetBuildingInfo))
		return;

	E_KTerBuildingType buildingType = GetType();
	WidgetBuildingInfo->SetBuildingIcon(UniqueId, buildingType, Status, RepairType, Level, CompleteTime, ProductTime);
}

TWeakObjectPtr<AKTerObserver> UKTerBuildingInfoComponent::getObserver()
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

void UKTerBuildingInfoComponent::updateWorldTreeAttribute()
{
	TSharedPtr<IKTerModel> modelSp = FKMvcHolder::Get().GetTerModel();
	if (!modelSp.IsValid())
		return;

	if (!TerBuildingInfoWp.IsValid())
		return;

	if (!TerBuildingInfoWp->IsWorldTree())
		return;

	const int32 regionId = modelSp->GetBelongingRegionId(UniqueId);

	const E_KTerAttributeType attributeType = modelSp->GetWorldTreeAttribute(regionId);

	SetWorldTreeAttribute(attributeType);
	SetWorldTreeAttributeVisibility(true);
}

void UKTerBuildingInfoComponent::onSequenceEndedCallback()
{
	TSharedPtr<IKTerView> viewSp = FKMvcHolder::Get().GetTerView();
	if (!viewSp.IsValid())
		return;

	viewSp->SpawnBuildingActor(UniqueId, Level);
	viewSp->ActivateDecoActor(UniqueId, Level);

	TSharedPtr<IKUiManager> uiManager = FKSystemHolder::Get().GetUiManager();
	if (!uiManager.IsValid())
		return;

	UKUiUnLockResult * result = Cast<UKUiUnLockResult>(uiManager->OpenMenu(TEXT("Hud/WB_UnLock_Result")));
	if (IsValid(result))
	{
		result->SetupUnLockResult(UniqueId);
	}
}

void UKTerBuildingInfoComponent::onLookChangeSequenceEndedCallback()
{
	TSharedPtr<IKTerView> viewSp = FKMvcHolder::Get().GetTerView();
	if (!viewSp.IsValid())
		return;

	viewSp->SpawnBuildingActor(UniqueId, Level);
	viewSp->ActivateDecoActor(UniqueId, Level);

	TSharedPtr<IKUiManager> uiManager = FKSystemHolder::Get().GetUiManager();
	if (!uiManager.IsValid())
		return;

	UKUITerBuildingUpgradeCompleteBox * menu = Cast<UKUITerBuildingUpgradeCompleteBox>(uiManager->OpenMenu("Hud/Manor/WB_Manor_Upgrade_Complete_Box"));
	if (menu)
	{
		menu->SetData(UniqueId);
	}
}

void UKTerBuildingInfoComponent::Init(TSharedPtr<const IKTerModelAgent>& modelAgent)
{
	if (!modelAgent.IsValid())
		return;

	UniqueId = modelAgent->GetUniqueId();
	Level = modelAgent->GetLevel();
	Status = modelAgent->GetStatus();
	PrevStatus = Status;
	RepairType = modelAgent->GetRepairType();
	CompleteTime = modelAgent->GetCompleteTime();
	ProductTime = modelAgent->GetProductTime();

	TSharedPtr<IKTerModel> modelSp = FKMvcHolder::Get().GetTerModel();
	if (!modelSp.IsValid())
		return;

	int32 regionId = modelSp->GetBelongingRegionId(UniqueId);

	if (modelSp->IsRegionOpened(regionId))
	{
		// 유아이 정보 셋팅
		initWidgetBuildingInfo();
	}

	// 세계수 속성 셋팅
	updateWorldTreeAttribute();
}

void UKTerBuildingInfoComponent::UpdateBuildinginfoComponent(TSharedPtr<const IKTerModelAgent>& modelAgent)
{
	if ( !modelAgent.IsValid() )
		return;

	UniqueId = modelAgent->GetUniqueId();
	Level = modelAgent->GetLevel();
	Status = modelAgent->GetStatus();
	RepairType = modelAgent->GetRepairType();
	CompleteTime = modelAgent->GetCompleteTime();
	ProductTime = modelAgent->GetProductTime();

	TSharedPtr<IKTerModel> modelSp = FKMvcHolder::Get().GetTerModel();
	if (!modelSp.IsValid())
		return;

	int32 regionId = modelSp->GetBelongingRegionId(UniqueId);

	if (modelSp->IsRegionOpened(regionId) && !IsValid(WidgetBuildingInfo))
	{
		// 유아이 정보 셋팅
		initWidgetBuildingInfo();
	}

	if (!IsValid(WidgetBuildingInfo))
		return;
	
	if ( PrevStatus != Status )
	{
		/** 언락커블에서 언락으로 바뀔때 자물쇠 연출해준다! */
		if ( PrevStatus == E_KTerBuildingStatus::UnLockable )
		{
			WidgetBuildingInfo->PlayUnlockAniBp();
		}

		PrevStatus = Status;
	}

	updateWidgetBuildingInfo();

	updateWorldTreeAttribute();
}

void UKTerBuildingInfoComponent::InitWidgetBuildingInfo()
{
	initWidgetBuildingInfo();
}

void UKTerBuildingInfoComponent::SetFocus()
{
	if (!getObserver().IsValid())
		return;

	getObserver()->SetDistrictFocus(GetType());
}

void UKTerBuildingInfoComponent::SetNew(bool isNew)
{
	if (!IsValid(WidgetBuildingInfo))
		return;

	WidgetBuildingInfo->SetNew(isNew);
}

bool UKTerBuildingInfoComponent::IsNew()
{
	if (IsValid(WidgetBuildingInfo))
	{
		return WidgetBuildingInfo->IsNew();
	}

	return false;
}

void UKTerBuildingInfoComponent::SetFree(bool isFree, int64 refreshTime /*= 0*/)
{
	if (!IsValid(WidgetBuildingInfo))
		return;

	WidgetBuildingInfo->SetNew(isFree);

	if (/*!isFree &&*/ refreshTime > 0)
	{
		WidgetBuildingInfo->SetFreeTimer(refreshTime);
	}
}

void UKTerBuildingInfoComponent::SetWorldTreeAttribute(E_KTerAttributeType attributeType)
{
	WorldTreeAttribute = attributeType;
}

void UKTerBuildingInfoComponent::SetWorldTreeAttributeVisibility(bool isVisible)
{
	if (TerBuildingInfoWp.IsValid())
	{
		TerBuildingInfoWp->SetWorldTreeAttrFxVisible(isVisible);
	}
}

E_KTerBuildingType UKTerBuildingInfoComponent::GetType()
{
	if (TerBuildingInfoWp.IsValid())
	{
		return TerBuildingInfoWp->GetType();
	}

	return E_KTerBuildingType::None;
}

void UKTerBuildingInfoComponent::UpdateLocationWidgetBuildingInfo(FVector location)
{
	if (!IsValid(WidgetBuildingInfo))
		return;

	if (getObserver().IsValid())
	{
		FVector offsetVector = FVector(0.f, 0.f, (getObserver()->GetBatchBuildingWidgetOffset() * getObserver()->GetRelativeZoomRate()));

		WidgetBuildingInfo->UpdateLocation(location + offsetVector);

		float zoomRate = FMath::Max(getObserver()->GetMinHUIZoomRate(), 1.0f - getObserver()->GetRelativeZoomRate());
		WidgetBuildingInfo->UpdateScale(zoomRate);

		WidgetBuildingInfo->UpdateEffect();
	}
}

FVector UKTerBuildingInfoComponent::GetOwnerActorLocation()
{
	if (TerBuildingInfoWp.IsValid())
	{	
		return TerBuildingInfoWp->GetActorLocation();
	}
	return FVector::ZeroVector;
}

void UKTerBuildingInfoComponent::OnAreaProduct(int32 itemTid, int64 acquired)
{
	if (IsValid(WidgetBuildingInfo))
	{
		WidgetBuildingInfo->OnAreaProduct(itemTid, acquired);
	}
}

bool UKTerBuildingInfoComponent::PlayEventUnlock()
{
	TSharedPtr<IKTerView> viewSp = FKMvcHolder::Get().GetTerView();
	if (!viewSp.IsValid())
		return false;

	if (!TerBuildingInfoWp.IsValid())
		return false;

	if (!Observer.IsValid())
		return false;

	/** 연출용 데이터가 없거나 연출하는 상황이 아니면 그냥 결과 팝업을 바로 호출한다. */
	if (TerBuildingInfoWp->GetOpenSequence() == nullptr ||
		!viewSp->DespawnBuildingActor(UniqueId, Level))
	{
		onSequenceEndedCallback();
		return false;
	}

	/** 연출용 데이터가 있다면*/
	Observer->SetOpenDirectingBuilding(TerBuildingInfoWp,
		FKActionDelegate::CreateUObject(this, &UKTerBuildingInfoComponent::onSequenceEndedCallback));

	return true;
}

bool UKTerBuildingInfoComponent::PlayEventLookChange(int32 lookChangeIndex)
{
	TSharedPtr<IKTerView> viewSp = FKMvcHolder::Get().GetTerView();
	if (!viewSp.IsValid())
		return false;

	if (!TerBuildingInfoWp.IsValid())
		return false;

	if (!Observer.IsValid())
		return false;

	/** 연출용 데이터가 없거나 연출하는 상황이 아니면 그냥 결과 팝업을 바로 호출한다. */
	if (TerBuildingInfoWp->GetLookChangeSequence(lookChangeIndex) == nullptr ||
		!viewSp->DespawnBuildingActor(UniqueId, Level))
	{
		onLookChangeSequenceEndedCallback();
		return false;
	}

	Observer->SetLookChangeDirectingBuilding(TerBuildingInfoWp, lookChangeIndex,
		FKActionDelegate::CreateUObject(this, &UKTerBuildingInfoComponent::onLookChangeSequenceEndedCallback));

	return true;
}

void UKTerBuildingInfoComponent::UpdateResearch(bool bisResearching, int32 tid, int64 startTime)
{	
	if (!IsValid(WidgetBuildingInfo))
		return;

	if (bisResearching)
	{
		WidgetBuildingInfo->OnResearchStarted(tid, startTime);
	}
	else
	{
		WidgetBuildingInfo->OnResearchEnded();
		updateWidgetBuildingInfo();
	}
}

