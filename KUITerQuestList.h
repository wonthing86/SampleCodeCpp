// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "System/UI/Core/KUiMainPanelMenu.h"
#include "KUITerQuestList.generated.h"

class APlayerController;
class UKUiTerQuestSlot;
class USizeBox;

/**
 * 
 */
UCLASS()
class KGAME_API UKUITerQuestList : public UKUiMainPanelMenu
{
	GENERATED_BODY()

public:
	void SetQuestList(int32 uniqueId, bool isUpgrade, bool isComplete);

	int32 GetUniqueId() { return UniqueId; }

	void SetListPosition(FVector2D position);

	void BlockClose();

	UFUNCTION(BlueprintCallable)
	void OnClickedClose();

	UFUNCTION(BlueprintImplementableEvent)
	void OnTutorialContentsReady();

	UUserWidget* GetQuestSlot(int32 questId);

private:
	bool bBlockClose = false;

	int32 UniqueId = 0;

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USizeBox * RootSizeBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UKUiTerQuestSlot * AgentQuestList;
};
