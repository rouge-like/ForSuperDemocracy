#pragma once

#include "CoreMinimal.h"
#include "Huxley/TerminidAnimInstance.h"
#include "TerminidTypes.h"
#include "ScavengerAnimInstance.generated.h"

class ATerminidScavenger;

UCLASS(BlueprintType, Blueprintable)
class FORSUPERDEMOCRACY_API UScavengerAnimInstance : public UTerminidAnimInstance
{
	GENERATED_BODY()

public:
	UScavengerAnimInstance();

protected:
	virtual void NativeInitializeAnimation() override;

	// 부모 클래스의 가상 함수들 오버라이드
	virtual void UpdateMovementVariables() override;
	virtual void UpdateCombatVariables() override;
	virtual void UpdateStateVariables() override;

	// Scavenger 전용 애니메이션 상태 확인 함수들 (부모 함수 오버라이드)
	virtual float GetMovementPlayRate() const override;

protected:
	// Scavenger 캐시된 참조 (부모의 TerminidOwner를 캐스팅한 것)
	UPROPERTY(BlueprintReadOnly, Category = "References|Scavenger")
	ATerminidScavenger* ScavengerOwner;

	// Scavenger 전용 변수들
	UPROPERTY(BlueprintReadOnly, Category = "Animation|Scavenger", meta = (AllowPrivateAccess = "true"))
	float AttackChargePercent;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Scavenger", meta = (AllowPrivateAccess = "true"))
	bool bIsAttackCharging;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Scavenger", meta = (AllowPrivateAccess = "true"))
	float TimeSinceLastAttack;

public:
	// Scavenger 전용 유틸리티 함수들
	UFUNCTION(BlueprintPure, Category = "Animation|Scavenger")
	bool IsChargingAttack() const { return bIsAttackCharging; }

	UFUNCTION(BlueprintPure, Category = "Animation|Scavenger")
	float GetAttackChargePercent() const { return AttackChargePercent; }

	UFUNCTION(BlueprintPure, Category = "Animation|Scavenger")
	bool ShouldPlayScavengerIdleVariation() const;

	UFUNCTION(BlueprintPure, Category = "Animation|Scavenger")
	bool ShouldPlayScavengerAttackCombo() const;

private:
	// Scavenger 전용 내부 계산용 변수들
	float LastAttackTime;
};