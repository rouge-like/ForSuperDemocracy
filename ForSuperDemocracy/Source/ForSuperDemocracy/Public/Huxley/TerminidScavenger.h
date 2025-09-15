#pragma once

#include "CoreMinimal.h"
#include "Huxley/TerminidBase.h"
#include "TerminidScavenger.generated.h"

UCLASS()
class FORSUPERDEMOCRACY_API ATerminidScavenger : public ATerminidBase
{
    GENERATED_BODY()

public:
    ATerminidScavenger();

protected:
    virtual void BeginPlay() override;

public:
    // 스캐빈저 고유 속성들
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scavenger", meta = (ClampMin = "0.1", ClampMax = "1.0"))
    float FleeHealthThreshold = 0.3f; // 도주 시작 체력 임계값

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scavenger", meta = (ClampMin = "200.0", ClampMax = "1000.0"))
    float FleeDistance = 500.0f; // 도주 시 이동 거리

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scavenger", meta = (ClampMin = "0.1", ClampMax = "3.0"))
    float AggressionBoost = 1.2f; // 군집 시 공격력 증가 배수

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scavenger", meta = (ClampMin = "100.0", ClampMax = "500.0"))
    float GroupDetectionRadius = 200.0f; // 다른 스캐빈저 감지 범위

    // 스캐빈저 특수 행동
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scavenger")
    bool bCanCallForHelp = true; // 죽을 때 근처 동료 호출 가능

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scavenger", meta = (ClampMin = "200.0", ClampMax = "800.0"))
    float HelpCallRadius = 400.0f; // 도움 요청 범위

    // 스캐빈저 상태 확인
    UFUNCTION(BlueprintPure, Category = "Scavenger")
    bool ShouldFleeFromCombat() const;

    UFUNCTION(BlueprintPure, Category = "Scavenger")
    bool IsInGroup() const;

    UFUNCTION(BlueprintPure, Category = "Scavenger")
    int32 GetNearbyScavengerCount() const;

    // 스캐빈저 전용 행동들
    UFUNCTION(BlueprintCallable, Category = "Scavenger")
    void CallForHelp();

    UFUNCTION(BlueprintCallable, Category = "Scavenger")
    void RespondToHelpCall(ATerminidScavenger* Caller);

protected:
    // 행동 오버라이드 - 스캐빈저 특화
    virtual void ProcessIdleBehavior(float DeltaTime) override;
    virtual void ProcessChaseBehavior(float DeltaTime) override;
    virtual void ProcessAttackBehavior(float DeltaTime) override;
    virtual void ProcessFleeBehavior(float DeltaTime) override;
    virtual void ProcessSwarmBehavior(float DeltaTime) override;

    // 전투 오버라이드
    virtual bool CanPerformAttack() const override;
    virtual void PerformAttack() override;
    virtual void Die() override;

    // 스캐빈저 전용 내부 함수들
    void UpdateFleeLogic(float DeltaTime);
    void UpdateGroupBehavior(float DeltaTime);
    void CheckForNearbyScavengers();
    
    // 이동 처리
    void ProcessQuickMovement(const FVector& TargetLocation, float DeltaTime);
    void ProcessIdleMovement(float DeltaTime);
    void ProcessZigzagChase(float DeltaTime);
    
    // 그룹 기반 공격력 계산
    float GetGroupAttackMultiplier() const;

private:
    // 내부 상태 변수들
    bool bIsInGroup;
    bool bHasCalledForHelp;
    float LastGroupCheckTime;
    int32 NearbyScavengerCount;
    
    // 도주 관련
    FVector FleeTargetLocation;
    bool bIsFleeingActive;
    float FleeStartTime;

    // Idle 상태 랜덤 이동 관련
    FVector IdleTargetLocation;
    bool bHasIdleTarget;
    float IdleTargetReachThreshold;

    // Chase 상태 지그재그 이동 관련
    FVector ZigzagTargetLocation;
    bool bIsZigzagging;
    float LastZigzagTime;

    // 그룹 멤버 참조
    UPROPERTY()
    TArray<ATerminidScavenger*> GroupMembers;
    
    // 내부 타이머들
    float LastHelpCallTime;
    
    // 상수들
    static constexpr float GROUP_CHECK_INTERVAL = 1.0f;
    static constexpr float HELP_CALL_COOLDOWN = 5.0f;
    static constexpr float FLEE_TIMEOUT = 10.0f;
};