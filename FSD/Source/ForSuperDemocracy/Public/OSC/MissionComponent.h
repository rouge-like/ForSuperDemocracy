// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OSC/MissionData.h"
#include "MissionComponent.generated.h"

class AActor;

// 진행 상태
UENUM(BlueprintType)
enum class EMissionState : uint8
{
    Idle,
    Running,
    Completed,
    Failed
};

// 델리게이트 정의
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveChanged, const FMissionObjective&, Objective);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveUpdated, int32, Curr, int32, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnObjectiveCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMissionCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMissionFailed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimerTick, int32, RemainSec);

/**
 * 미션 컴포넌트(싱글 기준)
 * - MainMode가 보유, MissionData로 초기화
 * - 한 맵 내 연속 목표( Kill -> Destroy -> ReachArea -> Survive ) 수행
 */
UCLASS(ClassGroup=(Game), meta=(BlueprintSpawnableComponent))
class FORSUPERDEMOCRACY_API UMissionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMissionComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void StartMission();

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void StopMission();

    // 외부 이벤트 훅
    UFUNCTION(BlueprintCallable, Category = "Mission|Notify")
    void NotifyKill(FName EnemyTag);

    UFUNCTION(BlueprintCallable, Category = "Mission|Notify")
    void NotifyDestroyed(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Mission|Notify")
    void NotifyAreaEntered(FName AreaTag);

    // 조회
    UFUNCTION(BlueprintPure, Category = "Mission")
    bool HasMission() const { return MissionData != nullptr && MissionData->Objectives.Num() > 0; }

    UFUNCTION(BlueprintPure, Category = "Mission")
    int32 GetObjectiveIndex() const { return ObjectiveIndex; }

    UFUNCTION(BlueprintPure, Category = "Mission")
    FMissionObjective GetCurrentObjective() const { return MissionData->Objectives[ObjectiveIndex]; };

    UFUNCTION(BlueprintPure, Category = "Mission")
    EMissionState GetState() const { return State; }

    // 델리게이트
    UPROPERTY(BlueprintAssignable, Category = "Mission|Event")
    FOnObjectiveChanged OnObjectiveChanged;

    UPROPERTY(BlueprintAssignable, Category = "Mission|Event")
    FOnObjectiveUpdated OnObjectiveUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Mission|Event")
    FOnObjectiveCompleted OnObjectiveCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Mission|Event")
    FOnMissionCompleted OnMissionCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Mission|Event")
    FOnMissionFailed OnMissionFailed;

    UPROPERTY(BlueprintAssignable, Category = "Mission|Event")
    FOnTimerTick OnTimerTick;

protected:
    // 목표 전환/타이머(내부)
    void StartObjective(int32 Index);
    void CompleteObjective();
    void FailObjective();

    UFUNCTION()
    void TickTimer(); // 1Hz 타이머 바인딩용

    // 데이터/상태
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
    TObjectPtr<UMissionData> MissionData = nullptr;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Mission")
    int32 ObjectiveIndex = INDEX_NONE;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Mission")
    int32 Curr = 0; // 진행도(처치/파괴 수 등)

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Mission")
    int32 RemainSec = 0; // 생존/제한시간 남은 초

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Mission")
    EMissionState State = EMissionState::Idle;

    // Destroy 중복 집계 방지
    UPROPERTY(Transient)
    TSet<TWeakObjectPtr<AActor>> CountedActors;

    FTimerHandle TimerHandle_1Hz;
};
