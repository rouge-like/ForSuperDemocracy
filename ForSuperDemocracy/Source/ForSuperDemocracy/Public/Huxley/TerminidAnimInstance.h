#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TerminidTypes.h"
#include "TerminidAnimInstance.generated.h"

class ATerminidBase;

UCLASS(BlueprintType, Blueprintable)
class FORSUPERDEMOCRACY_API UTerminidAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UTerminidAnimInstance();

protected:
    virtual void BeginPlay() override;
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

public:
    // Animation State Variables (Blueprint에서 사용)
    UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
    bool bIsSpawning;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
    bool bIsDead;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
    bool bIsAttacking;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
    bool bIsHurt;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
    bool bIsFleeing;

    // Movement Variables
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    bool bShouldMove;

    // Combat Variables
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    float HealthPercent;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    bool bHasTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    float DistanceToTarget;

    // FSM State
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FSM")
    ETerminidState CurrentState;

    // Terminid Type
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Type")
    ETerminidType TerminidType;

protected:
    // Owner Reference
    UPROPERTY(BlueprintReadOnly, Category = "Animation|References")
    ATerminidBase* TerminidOwner;

    // Animation Event Functions (Blueprint에서 오버라이드)
    UFUNCTION(BlueprintImplementableEvent, Category = "Animation|Events")
    void OnSpawnAnimationStart();

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation|Events")
    void OnSpawnAnimationComplete();

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation|Events")
    void OnAttackStart();

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation|Events")
    void OnAttackComplete();

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation|Events")
    void OnDeathStart();

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation|Events")
    void OnHurtReaction();

    // Animation Montage References
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Montages")
    class UAnimMontage* SpawnMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Montages")
    class UAnimMontage* AttackMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Montages")
    class UAnimMontage* DeathMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Montages")
    class UAnimMontage* HurtMontage;

private:
    void UpdateMovementVariables();
    void UpdateCombatVariables();
    void UpdateStateVariables();

#if WITH_EDITOR
public:
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};