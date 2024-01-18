// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KTerBuildingInfoComponent.generated.h"

class AKTerBuildingInfo;
class UKUITerBuildingInfo;
class IKTerModelAgent;
class AKTerObserver;

enum class E_KTerBuildingType : uint8;
enum class E_KTerBuildingMenuButtonType : uint8;
enum class E_KTerBuildingStatus : uint8;
enum class E_KTerBuildingRepairType : uint8;
enum class E_KTerAttributeType : uint8;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KGAME_API UKTerBuildingInfoComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UKTerBuildingInfoComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	TWeakObjectPtr<AKTerBuildingInfo> TerBuildingInfoWp;
	TWeakObjectPtr<AKTerObserver> Observer;

	UPROPERTY(VisibleAnywhere, Category = BuildingInfo)
	UKUITerBuildingInfo * WidgetBuildingInfo;

	/** 서버 데이터 셋 */
	int32 UniqueId;
	int32 Level;
	E_KTerBuildingStatus Status;
	E_KTerBuildingStatus PrevStatus;
	E_KTerBuildingRepairType RepairType;
	int64 CompleteTime;
	int64 ProductTime;

	// 세계수 속성
	E_KTerAttributeType WorldTreeAttribute = E_KTerAttributeType(0);

	/** Default Empty Array */
	TArray<E_KTerBuildingMenuButtonType> DefaultMenuButtons;

private:

	/** 헤드업 */
	void initWidgetBuildingInfo();

	void updateWidgetBuildingInfo();

	TWeakObjectPtr<AKTerObserver> getObserver();

	void updateWorldTreeAttribute();

	void onSequenceEndedCallback();

	void onLookChangeSequenceEndedCallback();

public:

	void Init(TSharedPtr<const IKTerModelAgent>& modelAgent);

	void UpdateBuildinginfoComponent(TSharedPtr<const IKTerModelAgent>& modelAgent);

	void InitWidgetBuildingInfo();

	void SetFocus();

	void SetNew(bool isNew);

	bool IsNew();

	void SetFree(bool isFree, int64 refreshTime = 0);

	/** 세계수 속성 설정 */
	void SetWorldTreeAttribute(E_KTerAttributeType attributeType);
	
	/** 세계수 속성 이펙트 액터 영지에서 Visibility */
	void SetWorldTreeAttributeVisibility(bool isVisible);

	/** 서버 테이터 셋으로 통합될 듯*/
	int32 GetUniqueId() const { return UniqueId; }
	int32 GetLevel() const { return Level; }
	E_KTerBuildingType GetType();
	E_KTerBuildingStatus GetStatus() const { return Status; }
	E_KTerBuildingRepairType GetRepairType() const { return RepairType; }
	int64 GetCompleteTime() const { return CompleteTime; }
	E_KTerAttributeType GetWorldTreeAttribute() const { return WorldTreeAttribute; }

	void UpdateLocationWidgetBuildingInfo(FVector location);

	FVector GetOwnerActorLocation();

	TWeakObjectPtr<AKTerBuildingInfo> GetTerBuildingInfo() { return TerBuildingInfoWp; }

	UKUITerBuildingInfo* GetTerBuildingInfoWidget() { return WidgetBuildingInfo; }

	/** 생산 성공 시 */
	void OnAreaProduct(int32 itemTid, int64 acquired);

	/** 개방 연출 */
	bool PlayEventUnlock();

	/** 외형 변화 연출 */
	bool PlayEventLookChange(int32 lookChangeIndex);

	/** 연구소 연구 HUI 갱신 */
	void UpdateResearch(bool bisResearching, int32 tid, int64 startTime);
};
