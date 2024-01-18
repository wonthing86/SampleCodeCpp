// Fill out your copyright notice in the Description page of Project Settings.

#include "KUITerQuestList.h"
#include "GameModule/KTer/Interface/IKTerView.h"
#include "GameModule/KTer/UI/KUiTerQuestSlot.h"
#include "GameModule/KTer/UI/KUiTerHud.h"
#include "GameModule/KTer/UI/KUiTerHelper.h"
#include "GameModule/KTer/KTerObserver.h"
#include "GameModule/KTer/KTerNpc.h"
#include "Components/SizeBox.h"
#include "Holder/KMvcHolder.h"
#include "Holder/KSystemHolder.h"
#include "System/Interface/IKUiManager.h"
#include "System/UI/Core/KUiBpHelper.h"

void UKUITerQuestList::SetQuestList(int32 uniqueId, bool isUpgrade, bool isComplete)
{
	if (AgentQuestList)
	{
		UniqueId = uniqueId;
		AgentQuestList->OpenList(uniqueId, isUpgrade, isComplete);
		AgentQuestList->QuestSlotCallback.BindUObject(this, &UKUITerQuestList::BlockClose);
	}
}

void UKUITerQuestList::SetListPosition(FVector2D position)
{
	TSharedPtr<IKTerView> viewSp = FKMvcHolder::Get().GetTerView();
	if (!viewSp.IsValid())
		return;

	TWeakObjectPtr<UKUiTerHud> terHud = viewSp->GetHud();
	if (!terHud.IsValid())
		return;

	float dpiScale = 1.0f;
	if (terHud.IsValid())
	{
		dpiScale = terHud->GetCurDPIScale();
	}

	if (AgentQuestList && RootSizeBox)
	{
		FVector2D viewportSize = FVector2D::ZeroVector;
		GetWorld()->GetGameViewport()->GetViewportSize(viewportSize);
		FVector2D fullSize = viewportSize * (1.0f / dpiScale);

		float newX = FMath::Min(position.X, fullSize.X - RootSizeBox->WidthOverride);
		float newY = FMath::Min(position.Y, fullSize.Y - RootSizeBox->HeightOverride);

		AgentQuestList->SetRenderTranslation(FVector2D(newX, FMath::Max(20.0f, newY)));
	}

	OnTutorialContentsReady();
}

void UKUITerQuestList::BlockClose()
{
	bBlockClose = true;
}

void UKUITerQuestList::OnClickedClose()
{
	if (bBlockClose)
		return;

	TSharedPtr<IKUiManager> uiManager = FKSystemHolder::Get().GetUiManager();
	if (uiManager.IsValid())
	{
		uiManager->SetHudVisibility(true);
	}

	Close();

	FKUiTerHelper::HiddenManorAreaOutline();

	TSharedPtr<IKTerView> viewSp = FKMvcHolder::Get().GetTerView();
	if (viewSp.IsValid())
	{
		viewSp->ResetNpcnCamera(UniqueId);
	}
}

UUserWidget* UKUITerQuestList::GetQuestSlot(int32 questId)
{
	if (IsValid(AgentQuestList))
		return AgentQuestList->GetQuestSlot(questId);

	return nullptr;
}
