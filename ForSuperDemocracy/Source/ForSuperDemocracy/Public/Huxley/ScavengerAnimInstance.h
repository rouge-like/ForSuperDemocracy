#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TerminidTypes.h"
#include "ScavengerAnimInstance.generated.h"

class ATerminidBase;

UCLASS()
class FORSUPERDEMOCRACY_API UScavengerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UScavengerAnimInstance();

protected:
	// 애니메이션 초기화
	virtual void NativeInitializeAnimation() override;
	
	// 매 프레임 애니메이션 업데이트
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	// 캐시된 참조
	UPROPERTY(BlueprintReadOnly, Category = "References")
	ATerminidBase* TerminidOwner;

	// 기본 이동 정보
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Direction;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsMoving;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsInAir;

	// 상태 정보
	UPROPERTY(BlueprintReadOnly, Category = "State")
	ETerminidState CurrentState;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsAlive;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsBurrowed;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsSpawning;

	// 전투 관련
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bHasTarget;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float DistanceToTarget;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsAttacking;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsHurt;

	// 체력 정보
	UPROPERTY(BlueprintReadOnly, Category = "Health")
	float HealthPercent;

	UPROPERTY(BlueprintReadOnly, Category = "Health")
	bool bShouldFlee;

private:
	// 내부 계산용
	FVector LastFrameVelocity;
	float MovementThreshold;

public:
	// 블루프린트에서 호출 가능한 유틸리티 함수들
	UFUNCTION(BlueprintPure, Category = "Animation")
	bool ShouldPlayIdleAnimation() const;

	UFUNCTION(BlueprintPure, Category = "Animation")
	bool ShouldPlayMoveAnimation() const;

	UFUNCTION(BlueprintPure, Category = "Animation")
	bool ShouldPlayAttackAnimation() const;

	UFUNCTION(BlueprintPure, Category = "Animation")
	bool ShouldPlayHurtAnimation() const;

	UFUNCTION(BlueprintPure, Category = "Animation")
	bool ShouldPlayDeathAnimation() const;

	UFUNCTION(BlueprintPure, Category = "Animation")
	bool ShouldPlayBurrowAnimation() const;

	UFUNCTION(BlueprintPure, Category = "Animation")
	bool ShouldPlayEmergeAnimation() const;

	UFUNCTION(BlueprintPure, Category = "Animation")
	float GetMovementPlayRate() const;
};