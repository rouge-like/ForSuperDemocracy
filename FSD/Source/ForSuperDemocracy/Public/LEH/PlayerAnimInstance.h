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
	UPlayerAnimInstance();
	
	virtual void NativeBeginPlay() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

public:
	// Player state
	UPROPERTY(BlueprintReadOnly)
	EPlayerState _PlayerState;
	
	UPROPERTY(BlueprintReadOnly)
	UPlayerFSM* PlayerFSM;
	
	
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

	// Current weapon
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentWeapon;

	// 던질 준비 애니메이션
	UPROPERTY(BlueprintReadOnly)
	bool bStartThrowAim;

	// 피격 애니메이션
	UPROPERTY(BlueprintReadOnly)
	bool bDamage;
public:
	// Anim montage
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=PlayerAnim)
	class UAnimMontage* ReloadMontage;

	void PlayReloadAnimation();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=PlayerAnim)
	class UAnimMontage* FireMontage;
	
	void PlayFireAnimation();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=PlayerAnim)
	class UAnimMontage* SaluteMontage;

	void PlaySaluteAnimation();
	void StopCurrentAnimation();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=PlayerAnim)
	class UAnimMontage* ThrowMontage;
	
	void PlayThrowAnimation();
};
