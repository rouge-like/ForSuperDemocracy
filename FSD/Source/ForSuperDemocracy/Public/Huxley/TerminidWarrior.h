#pragma once

#include "CoreMinimal.h"
#include "Huxley/TerminidBase.h"
#include "TerminidWarrior.generated.h"

UCLASS()
class FORSUPERDEMOCRACY_API ATerminidWarrior : public ATerminidBase
{
    GENERATED_BODY()

public:
    ATerminidWarrior();

protected:
    virtual void BeginPlay() override;

public:
    // 워리어 고유 속성들
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warrior", meta = (ClampMin = "0.3", ClampMax = "0.8"))
    float BerserkHealthThreshold = 0.5f; // 광폭화 시작 체력 임계값

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warrior", meta = (ClampMin = "1.0", ClampMax = "3.0"))
    float BerserkAttackMultiplier = 1.2f; // 광폭화 시 공격력 증가 배수

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warrior", meta = (ClampMin = "1.0", ClampMax = "2.0"))
    float BerserkSpeedMultiplier = 1.1f; // 광폭화 시 이동속도 증가 배수

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warrior", meta = (ClampMin = "200.0", ClampMax = "600.0"))
    float SwarmCoordinationRadius = 300.0f; // 군집 협력 범위

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warrior", meta = (ClampMin = "2", ClampMax = "8"))
    int32 OptimalSwarmSize = 4; // 최적 군집 크기

    // 워리어 전술 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warrior")
    bool bCanLeadSwarm = true; // 군집 리더 가능 여부

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warrior")
    bool bUseTacticalMovement = true; // 전술적 이동 사용

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warrior", meta = (ClampMin = "100.0", ClampMax = "400.0"))
    float FlankingDistance = 200.0f; // 측면 공격 거리

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warrior", meta = (ClampMin = "1.0", ClampMax = "10.0"))
    float LeadershipRange = 400.0f; // 리더십 범위 (다른 Terminid에게 영향)

    // 워리어 상태 확인
    UFUNCTION(BlueprintPure, Category = "Warrior")
    bool IsBerserk() const;

    UFUNCTION(BlueprintPure, Category = "Warrior")
    bool IsSwarmLeader() const;

    UFUNCTION(BlueprintPure, Category = "Warrior")
    int32 GetSwarmMemberCount() const;

    UFUNCTION(BlueprintPure, Category = "Warrior")
    bool HasOptimalSwarmSize() const;

    // 워리어 전용 행동들
    UFUNCTION(BlueprintCallable, Category = "Warrior")
    void EnterBerserkMode();

    UFUNCTION(BlueprintCallable, Category = "Warrior")
    void ExitBerserkMode();

    UFUNCTION(BlueprintCallable, Category = "Warrior")
    void BecomeSwarmLeader();

    UFUNCTION(BlueprintCallable, Category = "Warrior")
    void CoordinateSwarmAttack();

    UFUNCTION(BlueprintCallable, Category = "Warrior")
    FVector CalculateFlankingPosition(AActor* Target) const;

protected:
    // 행동 오버라이드 - 워리어 특화
    virtual void ProcessIdleBehavior(float DeltaTime) override;
    virtual void ProcessPatrolBehavior(float DeltaTime) override;
    virtual void ProcessChaseBehavior(float DeltaTime) override;
    virtual void ProcessAttackBehavior(float DeltaTime) override;
    virtual void ProcessSwarmBehavior(float DeltaTime) override;

    // 전투 오버라이드
    virtual bool CanPerformAttack() const override;
    virtual void PerformAttack() override;
    virtual void Die() override;

    // 워리어 전용 내부 함수들
    void UpdateBerserkState(float DeltaTime);
    void UpdateSwarmLeadership(float DeltaTime);
    void UpdateTacticalMovement(float DeltaTime);
    
    // 군집 관리
    void CheckForSwarmMembers();
    void IssueSwarmCommands();
    void CalculateSwarmFormation();
    
    // 전술적 이동
    void ExecuteFlankingManeuver(float DeltaTime);
    void ExecuteCoordinatedAttack(float DeltaTime);
    
    // 리더십 효과
    void ApplyLeadershipBonus();
    void BoostNearbyTerminids();

private:
    // 내부 상태 변수들
    bool bIsBerserk;
    bool bIsSwarmLeader;
    float BerserkActivationTime;
    float LastSwarmCheckTime;
    
    // 군집 관련
    int32 SwarmMemberCount;
    FVector SwarmFormationCenter;
    float SwarmFormationRadius;
    
    // 전술 관련
    bool bIsExecutingFlankManeuver;
    FVector FlankingTarget;
    float LastTacticalUpdateTime;
    
    // 군집 멤버 참조
    UPROPERTY()
    TArray<ATerminidBase*> SwarmMembers;
    
    // 리더십 대상
    UPROPERTY()
    TArray<ATerminidBase*> LeadershipTargets;
    
    // 내부 타이머들
    float LastLeadershipUpdateTime;
    float LastCoordinationTime;
    
    // 상수들
    static constexpr float SWARM_CHECK_INTERVAL = 1.5f;
    static constexpr float TACTICAL_UPDATE_INTERVAL = 0.5f;
    static constexpr float LEADERSHIP_UPDATE_INTERVAL = 2.0f;
    static constexpr float COORDINATION_COOLDOWN = 3.0f;
    static constexpr float BERSERK_MIN_DURATION = 5.0f;
};