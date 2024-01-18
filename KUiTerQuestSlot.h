// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "System/UI/Core/KUiPanelMenu.h"
#include "KUiTerQuestSlot.generated.h"

class UKUiTerQuestSlotChild;
class FKAcuBriefingBase;

enum class E_KQuestAgentType : uint8;
enum class E_KAcuBfReply : uint8;

DECLARE_DELEGATE(FKTerQuestSlotCallback);

/**
 * bp :WB_ComHud_HDI_Btn_List
 */
UCLASS()
class UKUiTerQuestSlot : public UKUiPopupMenu
{
	GENERATED_BODY()

public:
	FKTerQuestSlotCallback QuestSlotCallback;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSoftClassPtr<UKUiTerQuestSlotChild> SlotClassPtr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSoftClassPtr<UKUiTerUpgradeSlotChild> UpgradeSlotClassPtr;

	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* SlotBox;

	int32 UniqueId;

	UPROPERTY()
	TArray<UKUiTerQuestSlotChild*> SlotArr;

	UPROPERTY()
	UKUiTerUpgradeSlotChild * UpgradeSlot = nullptr;

public:
	void OpenList(int32 uniqueId, bool isUpgrade, bool isComplete);

	UFUNCTION(BlueprintPure)
	UUserWidget* GetUpgradeSlot(){ return Cast<UUserWidget>(UpgradeSlot); }

	UFUNCTION(BlueprintPure)
	UUserWidget* GetQuestSlot(int32 questId);

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void playShowAnim();

	UFUNCTION()
	void onClickSlot(class UKUiTerQuestSlotChild* clickedSlot);

	UFUNCTION()
	void onClickUpgradeSlot(class UKUiTerUpgradeSlotChild* clickedSlot);

	UFUNCTION()
	void onPreClickSlot(class UKUiTerQuestSlotChild* clickedSlot);

	UFUNCTION()
	void onPreClickUpgradeSlot(class UKUiTerUpgradeSlotChild* clickedSlot);
};

DECLARE_DELEGATE_OneParam(FKTerQuestSlotClicked, UKUiTerQuestSlotChild*);

UCLASS()
class UKUiTerQuestSlotChild : public UUserWidget
{
	GENERATED_BODY()

public:
	int32 UniqueId;
	int32 QuestTid;
	E_KQuestAgentType QuestAgentType;

	FKTerQuestSlotClicked SlotClickEvent;
	FKTerQuestSlotClicked SlotPreClickEvent;

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UKUiQuestImg* QuestImg;

public:
	void InitSlot(int32 uniqueId, int32 questTid, E_KQuestAgentType questAgentType);

	UFUNCTION(BlueprintImplementableEvent)
	void PlayCollapseAnim();

	UFUNCTION(BlueprintCallable)
	void OnSlotClicked();

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void setData_Bp(const FString& questName);

	UFUNCTION(BlueprintCallable)
	void onSlotPreclicked();
};

DECLARE_DELEGATE_OneParam(FKTerUpgradeSlotClicked, UKUiTerUpgradeSlotChild*);

UCLASS()
class UKUiTerUpgradeSlotChild : public UUserWidget
{
	GENERATED_BODY()

public:
	int32 UniqueId;
	int32 LinkedAreaId;

	FKTerUpgradeSlotClicked SlotClickEvent;
	FKTerUpgradeSlotClicked SlotPreClickEvent;

public:
	void InitSlot(int32 uniqueId, bool isComplete);

	UFUNCTION(BlueprintImplementableEvent)
	void PlayCollapseAnim();

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void setData_Bp(const FString& questName, bool isComplete);

	UFUNCTION(BlueprintCallable)
	void onSlotClicked();

	UFUNCTION(BlueprintCallable)
	void onSlotPreclicked();
};
