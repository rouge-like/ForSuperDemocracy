#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Engine/Engine.h"
#include "TerminidTypes.h"
#include "TerminidBase.generated.h"

class UTerminidFSM;
class ATerminidSpawner;

UCLASS(Abstract, BlueprintType, Blueprintable)
class FORSUPERDEMOCRACY_API ATerminidBase : public ACharacter
{   
    GENERATED_BODY()

public:
    ATerminidBase();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // AI 및 FSM 컴포넌트들
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UTerminidFSM* StateMachine;

    // 스탯 및 속성
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
    FTerminidStats BaseStats;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    ETerminidType TerminidType;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawner")
    ATerminidSpawner* ParentSpawner;

    // 타겟 및 감지
    UPROPERTY(BlueprintReadOnly, Category = "AI")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    FVector LastKnownTargetLocation;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    bool bHasTarget;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    float DistanceToTarget;

    // 전투 관련 속성
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float LastAttackTime;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bCanAttack;

    // 애니메이션 관련 속성
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    bool bIsSpawning;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    bool bIsPlayingAnimation;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
    float SpawnAnimationDuration;

    // 초기화
    UFUNCTION(BlueprintCallable, Category = "Terminid")
    void InitializeTerminid(const FTerminidStats& Stats, ETerminidType Type);

    UFUNCTION(BlueprintCallable, Category = "Terminid")
    void SetParentSpawner(ATerminidSpawner* Spawner);

    // 핵심 행동 함수들 - 파생 클래스에서 오버라이드
    UFUNCTION(BlueprintImplementableEvent, Category = "Behavior")
    void ExecuteIdleBehavior();

    UFUNCTION(BlueprintImplementableEvent, Category = "Behavior")
    void ExecutePatrolBehavior();

    UFUNCTION(BlueprintImplementableEvent, Category = "Behavior")
    void ExecuteChaseBehavior();

    UFUNCTION(BlueprintImplementableEvent, Category = "Behavior")
    void ExecuteAttackBehavior();

    UFUNCTION(BlueprintImplementableEvent, Category = "Behavior")
    void ExecuteHurtBehavior();

    UFUNCTION(BlueprintImplementableEvent, Category = "Behavior")
    void ExecuteSwarmBehavior();

    UFUNCTION(BlueprintImplementableEvent, Category = "Behavior")
    void ExecuteFleeBehavior();

    // 애니메이션 이벤트들 (Blueprint에서 구현)
    UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
    void OnSpawnAnimationStart();

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
    void OnSpawnAnimationComplete();

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
    void OnIdleAnimation();

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
    void OnMoveAnimation();

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
    void OnAttackAnimation();

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
    void OnDeathAnimation();

    // 스폰 관련 함수들
    UFUNCTION(BlueprintCallable, Category = "Spawning")
    void StartSpawnSequence();

    UFUNCTION(BlueprintCallable, Category = "Spawning")
    void CompleteSpawnSequence();

    UFUNCTION(BlueprintPure, Category = "Spawning")
    bool IsSpawning() const { return bIsSpawning; }
    
    virtual void ProcessIdleBehavior(float DeltaTime);
    virtual void ProcessPatrolBehavior(float DeltaTime);
    virtual void ProcessChaseBehavior(float DeltaTime);
    virtual void ProcessAttackBehavior(float DeltaTime);
    virtual void ProcessHurtBehavior(float DeltaTime);
    virtual void ProcessSwarmBehavior(float DeltaTime);
    virtual void ProcessFleeBehavior(float DeltaTime);
    virtual void ProcessDeathBehavior(float DeltaTime);

    // 타겟 관리
    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetCurrentTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void ClearTarget();

    UFUNCTION(BlueprintPure, Category = "AI")
    bool HasValidTarget() const;

    UFUNCTION(BlueprintCallable, Category = "AI")
    void UpdateTargetDistance();

    UFUNCTION(BlueprintPure, Category = "AI")
    bool IsTargetInRange(float Range) const;

    UFUNCTION(BlueprintPure, Category = "AI")
    bool IsTargetInAttackRange() const;

    // 전투
    UFUNCTION(BlueprintCallable, Category = "Combat")
    virtual bool CanPerformAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    virtual void PerformAttack();

    // Unreal Engine TakeDamage 오버라이드
    UFUNCTION()
    void OnDamaged(float Damage, AActor* DamageCauser, AController* EventInstigator, TSubclassOf<UDamageType> DamageType);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
    class UHealthComponent* Health;

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    void OnDamageReceived(float DamageAmount, AActor* DamageSource);

    // HealthComponent OnDeath 이벤트 핸들러
    UFUNCTION()
    void OnHealthComponentDeath(AActor* Victim);

    // 이동 유틸리티
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void MoveTowardsTarget(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void MoveTowardsLocation(const FVector& TargetLocation, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StopMovement();

    // 죽음 및 생명주기
    UFUNCTION(BlueprintCallable, Category = "Lifecycle")
    virtual void Die();

    UFUNCTION(BlueprintImplementableEvent, Category = "Lifecycle")
    void OnDeath();

    // 플레이어 감지 (기본 FSM 방식)
    UFUNCTION(BlueprintCallable, Category = "AI")
    void UpdatePlayerDetection(float DeltaTime);
    
    UFUNCTION(BlueprintPure, Category = "AI")
    APawn* FindNearestPlayer() const;
    
    UFUNCTION(BlueprintPure, Category = "AI")
    bool IsPlayerInSight(APawn* Player) const;
    
    UFUNCTION(BlueprintPure, Category = "AI")
    bool IsPlayerInDetectionRange(APawn* Player) const;

    // 유틸리티 함수들 - HealthComponent 기반
    UFUNCTION(BlueprintPure, Category = "Stats")
    float GetCurrentHealth() const;

    UFUNCTION(BlueprintPure, Category = "Stats")
    float GetMaxHealth() const;

    UFUNCTION(BlueprintPure, Category = "Stats")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Stats")
    bool IsAlive() const;

    UFUNCTION(BlueprintPure, Category = "AI")
    FORCEINLINE bool ShouldFlee() const { return GetHealthPercent() < 0.3f; }

    UFUNCTION(BlueprintPure, Category = "Stats")
    FORCEINLINE ETerminidType GetTerminidType() const { return TerminidType; }

    UFUNCTION(BlueprintPure, Category = "AI")
    FORCEINLINE AActor* GetCurrentTarget() const { return CurrentTarget; }

private:
    void UpdateMovement(float DeltaTime);
    void HandleDeath();
    
    // 플레이어 감지 관련
    float LastPlayerDetectionTime;
    static constexpr float PLAYER_DETECTION_INTERVAL = 0.5f; // 0.5초마다 감지 체크
    
    // 스폰 타이머 핸들
    FTimerHandle SpawnTimerHandle;

#if WITH_EDITOR
public:
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};