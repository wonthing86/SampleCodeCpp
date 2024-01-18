// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameModule/KTer/KTerDef.h"
#include "KTerNpcInfoComponent.generated.h"

class UKUITerHuiNpc;
class IKTerModelAgentNpc;
class FKAcuBriefingBase;
class AKTerObserver;

enum class E_KQuestAgentType : uint8;
enum class E_KQuestCategory : uint8;
enum class E_KAcuBfReply : uint8;
enum class E_KUiFrameBtnResult : uint8;
enum class E_KTalkConditionType : uint8;

DECLARE_DELEGATE_OneParam(FKPopupTooltipResultEvent, E_KUiFrameBtnResult);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KGAME_API UKTerNpcInfoComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UKTerNpcInfoComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	TWeakObjectPtr<AKTerObserver> Observer;

	TWeakObjectPtr<UKUITerHuiNpc> HeadUpWidget;

	int32 NpcUniqueId = 0;

	FString NpcName;

	int32 BubbleTalkGroupId = 0;

	E_KQuestAgentType HeadUpQuestType;

	bool bIsVisible = true;

protected:
	int32 QuestAutoTid = 0;

	int32 QuestAcceptTid = 0;

	FKPopupTooltipResultEvent QuestAcceptPopupDelegate;

private:
	void initHeadUp();

	void resetCamera(bool isHudVisible = true);

	void onQuestAcceptPanelClicked(E_KUiFrameBtnResult value);

	void reqQuestAccept(int32 questTid, int32 uniqueId);

	E_KAcuBfReply ackQuestAccept(TSharedPtr<FKAcuBriefingBase> bfBase);

	void reqQuestComplete(int32 questTid);

	E_KAcuBfReply ackQuestComplete(TSharedPtr<FKAcuBriefingBase> bfBase);

	void reqQuestNpcTalk(int32 questTid);

	E_KAcuBfReply ackQuestNpcTalk(TSharedPtr<FKAcuBriefingBase> bfBase);

	void checkBuildingUnlock(int32 questTid);

	void doQuestAutoBeforeAcceptPost();

	TWeakObjectPtr<AKTerObserver> getObserver();

	void hiddenAreaOutline();

public:
	void Init(TSharedPtr<const IKTerModelAgentNpc>& data);

	void UpdateLocationHeadUp(const FVector& location);

	// #swshin : Npc의 Gesture와 동기화 하기위해 Index로 특정 TalkContents 실행
	void PlayIdleTalk(int32 talkIndex);

	void SetSpecialTalk(bool isOn);

	void AddQuestEnderTalk(int32 questTid, const FString& talk);

	void RemoveQuestEnderTalk(int32 questTid);

	void AddTalkConditionId(E_KTalkConditionType conditionType, int32 conditionId);

	void RemoveTalkConditionId(E_KTalkConditionType conditionType, int32 conditionId);

	void SetQuest(E_KQuestAgentType nowQuestAgentType, E_KQuestCategory nowQuestCategory);

	void SetUpgrade(bool isUpgrade, int64 completeTime);

	void OpenQuestList();

	void OpenQuestAcceptPanel(int32 questTid);

	void PlayEventBeforeAccept(int32 questTid);

	void PlayEventAfterAccept(int32 questTid);

	void PlayEventComplete(int32 questTid);

	void PlayEventProgress(int32 questTid);

	void PlayEventUpgrade(int32 areaId);

	void DoQuestBeforeAutoAccept(int32 questTid);

	void DoQuestAutoAccept(int32 questTid);

	void DoQuestAutoComplete(int32 questTid);

	void ReqQuestComplete(int32 questTid);

	void ReqQuestNpcTalk(int32 questTid);

	int32 GetNpcUniqueId() const { return NpcUniqueId; }

	void RemoveWidget();

	void SetFocus();

	void SetHeadUpVisibility(bool bisVisible);

	bool IsIdleTalkState();

	TWeakObjectPtr<UKUITerHuiNpc> GetHeadUpWidget(){ return HeadUpWidget; }
};
