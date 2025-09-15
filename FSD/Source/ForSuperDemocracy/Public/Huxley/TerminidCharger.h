#pragma once

#include "CoreMinimal.h"
#include "Huxley/TerminidBase.h"
#include "TerminidCharger.generated.h"

UCLASS()
class FORSUPERDEMOCRACY_API ATerminidCharger : public ATerminidBase
{
    GENERATED_BODY()

public:
    ATerminidCharger();

protected:
    virtual void BeginPlay() override;

public:
    // 차저 고유 속성들
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charger", meta = (ClampMin = "500.0", ClampMax = "2000.0"))
    float ChargeDistance = 800.0f; // 돌진 공격 거리

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charger", meta = (ClampMin = "1.5", ClampMax = "5.0"))
    float ChargeSpeedMultiplier = 3.0f; // 돌진 시 속도 배수

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charger", meta = (ClampMin = "1.5", ClampMax = "4.0"))
    float ChargeDamageMultiplier = 2.5f; // 돌진 공격 데미지 배수

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charger", meta = (ClampMin = "100.0", ClampMax = "400.0"))
    float ChargeStunRadius = 200.0f; // 돌진 충격 범위

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charger", meta = (ClampMin = "2.0", ClampMax = "10.0"))
    float ChargeCooldown = 5.0f; // 돌진 공격 쿨다운

    // 방어 관련
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charger", meta = (ClampMin = "0.1", ClampMax = "0.8"))
    float ArmorDamageReduction = 0.3f; // 피해 감소율

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charger", meta = (ClampMin = "0.1", ClampMax = "1.0"))
    float HurtResistance = 0.7f; // 스턴 저항력

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charger")
    bool bCanBreakWalls = true; // 벽 파괴 가능

    // 위협도 관련
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charger", meta = (ClampMin = "300.0", ClampMax = "800.0"))
    float ThreatRadius = 500.0f; // 위협 감지 범위

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charger")
    bool bCausesFearEffect = true; // 공포 효과 유발

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charger", meta = (ClampMin = "200.0", ClampMax = "600.0"))
    float FearEffectRadius = 300.0f; // 공포 효과 범위

    // 차저 상태 확인
    UFUNCTION(BlueprintPure, Category = "Charger")
    bool IsCharging() const;

    UFUNCTION(BlueprintPure, Category = "Charger")
    bool CanPerformCharge() const;

    UFUNCTION(BlueprintPure, Category = "Charger")
    bool IsInChargeRange() const;

    UFUNCTION(BlueprintPure, Category = "Charger")
    float GetChargeDistanceToTarget() const;

    // 차저 전용 행동들
    UFUNCTION(BlueprintCallable, Category = "Charger")
    void StartChargeAttack();

    UFUNCTION(BlueprintCallable, Category = "Charger")
    void StopChargeAttack();

    UFUNCTION(BlueprintCallable, Category = "Charger")
    void ExecuteChargeImpact();

    UFUNCTION(BlueprintCallable, Category = "Charger")
    void CauseFearEffect();

    UFUNCTION(BlueprintCallable, Category = "Charger")
    void BreakNearbyWalls();

protected:
    // 행동 오버라이드 - 차저 특화
    virtual void ProcessIdleBehavior(float DeltaTime) override;
    virtual void ProcessChaseBehavior(float DeltaTime) override;
    virtual void ProcessAttackBehavior(float DeltaTime) override;
    virtual void ProcessHurtBehavior(float DeltaTime) override;

    // 전투 오버라이드
    virtual bool CanPerformAttack() const override;
    virtual void PerformAttack() override;
    virtual void Die() override;

    // 데미지 시스템 오버라이드
    virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

    // 차저 전용 내부 함수들
    void UpdateChargeAttack(float DeltaTime);
    void UpdateThreatDetection(float DeltaTime);
    void ProcessChargeMovement(float DeltaTime);
    
    // 돌진 관련
    void CalculateChargeTarget();
    void ExecuteChargePhysics(float DeltaTime);
    void HandleChargeCollision();
    
    // 공포 효과
    void ApplyFearToNearbyEnemies();
    void InfluenceNearbyTerminids();
    
    // 환경 상호작용
    void CheckWallBreaking();
    void DestroyEnvironmentObjects();

private:
    // 내부 상태 변수들
    bool bIsCharging;
    bool bChargeOnCooldown;
    float ChargeStartTime;
    float LastChargeTime;
    FVector ChargeStartLocation;
    FVector ChargeTargetLocation;
    
    // 돌진 상태 추적
    float ChargeCurrentSpeed;
    bool bHasHitTarget;
    bool bChargeCompleted;
    
    // 위협 및 공포 효과
    float LastThreatCheckTime;
    float LastFearEffectTime;
    bool bIsCausingFear;
    
    // 환경 파괴
    float LastWallCheckTime;
    TArray<AActor*> DestroyedObjects;
    
    // 타이머 관련
    FTimerHandle ChargeRecoveryTimerHandle;
    FTimerHandle FearEffectTimerHandle;
    
    // 상수들
    static constexpr float THREAT_CHECK_INTERVAL = 2.0f;
    static constexpr float FEAR_EFFECT_INTERVAL = 3.0f;
    static constexpr float WALL_CHECK_INTERVAL = 0.5f;
    static constexpr float CHARGE_ACCELERATION = 2000.0f;
    static constexpr float CHARGE_MIN_DISTANCE = 300.0f;
    static constexpr float CHARGE_IMPACT_FORCE = 1000.0f;
    static constexpr float POST_CHARGE_STUN_DURATION = 1.0f;

public:
    // Blueprint에서 사용할 이벤트들
    UFUNCTION(BlueprintImplementableEvent, Category = "Charger")
    void OnChargeStart();

    UFUNCTION(BlueprintImplementableEvent, Category = "Charger")
    void OnChargeImpact(AActor* HitActor);

    UFUNCTION(BlueprintImplementableEvent, Category = "Charger")
    void OnChargeEnd();

    UFUNCTION(BlueprintImplementableEvent, Category = "Charger")
    void OnWallBreak(AActor* BrokenWall);

    UFUNCTION(BlueprintImplementableEvent, Category = "Charger")
    void OnFearEffectTriggered();
};