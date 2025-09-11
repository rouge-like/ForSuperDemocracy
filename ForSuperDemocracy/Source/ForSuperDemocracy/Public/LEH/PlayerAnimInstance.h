// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerFSM.h"
#include "PlayerAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class FORSUPERDEMOCRACY_API UPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

public:
	// Player state
	UPROPERTY(BlueprintReadOnly)
	EPlayerState _PlayerState;
	
	// 플레이어 속도
    UPROPERTY(BlueprintReadOnly)
    float Speed;
    
    // 플레이어 방향
    UPROPERTY(BlueprintReadOnly)
    float Direction;
	
	// Aiming
	UPROPERTY(BlueprintReadOnly)
	bool bIsAiming;

	UPROPERTY(BlueprintReadOnly)
	FRotator AimingLocation; 
	
};
