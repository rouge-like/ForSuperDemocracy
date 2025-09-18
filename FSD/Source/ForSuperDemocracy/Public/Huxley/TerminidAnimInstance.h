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
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

public:
	// Animation State Variables (Blueprint에서 사용)
	UPROPERTY(BlueprintReadOnly, Category = "Animation|State", meta = (AllowPrivateAccess = "true"))
	bool bIsSpawning;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|State", meta = (AllowPrivateAccess = "true"))
	bool bIsDead;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|State", meta = (AllowPrivateAccess = "true"))
	bool bIsAttacking;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|State", meta = (AllowPrivateAccess = "true"))
	bool bIsHurt;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|State", meta = (AllowPrivateAccess = "true"))
	bool bIsFleeing;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|State", meta = (AllowPrivateAccess = "true"))
	bool bIsAlive;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|State", meta = (AllowPrivateAccess = "true"))
	bool bShouldFlee;

	// Movement Variables
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement", meta = (AllowPrivateAccess = "true"))
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement", meta = (AllowPrivateAccess = "true"))
	float Direction;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsMoving;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement", meta = (AllowPrivateAccess = "true"))
	bool bShouldMove;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;

	// Combat Variables
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat", meta = (AllowPrivateAccess = "true"))
	float HealthPercent;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat", meta = (AllowPrivateAccess = "true"))
	bool bHasTarget;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat", meta = (AllowPrivateAccess = "true"))
	float DistanceToTarget;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat", meta = (AllowPrivateAccess = "true"))
	bool bIsPlayingAttackAnimation;

	// FSM State
	UPROPERTY(BlueprintReadOnly, Category = "Animation|FSM", meta = (AllowPrivateAccess = "true"))
	ETerminidState CurrentState;

	// Terminid Type
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Type", meta = (AllowPrivateAccess = "true"))
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

	// 애니메이션 상태 확인 함수들 (자식 클래스에서 오버라이드 가능)
	UFUNCTION(BlueprintPure, Category = "Animation")
	virtual bool ShouldPlayIdleAnimation() const;

	UFUNCTION(BlueprintPure, Category = "Animation")
	virtual bool ShouldPlayMoveAnimation() const;

	UFUNCTION(BlueprintPure, Category = "Animation")
	virtual bool ShouldPlayAttackAnimation() const;

	UFUNCTION(BlueprintPure, Category = "Animation")
	virtual bool ShouldPlayHurtAnimation() const;

	UFUNCTION(BlueprintPure, Category = "Animation")
	virtual bool ShouldPlayDeathAnimation() const;

	UFUNCTION(BlueprintPure, Category = "Animation")
	virtual float GetMovementPlayRate() const;

	// Animation Montage References
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Montages")
	class UAnimMontage* SpawnMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Montages")
	class UAnimMontage* AttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Montages")
	class UAnimMontage* DeathMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Montages")
	class UAnimMontage* HurtMontage;

	// 가상 함수로 만들어 자식 클래스에서 확장 가능
	virtual void UpdateMovementVariables();
	virtual void UpdateCombatVariables();
	virtual void UpdateStateVariables();

private:
	// 내부 계산용 변수들
	float MovementThreshold;

#if WITH_EDITOR

public:
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
