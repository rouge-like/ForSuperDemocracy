// Fill out your copyright notice in the Description page of Project Settings.


#include "LEH/PlayerAnimInstance.h"

#include "GameFramework/PawnMovementComponent.h"
#include "LEH/PlayerCharacter.h"
#include "LEH/PlayerFSM.h"
#include "LEH/SuperPlayerController.h"


UPlayerAnimInstance::UPlayerAnimInstance()
{
	ConstructorHelpers::FObjectFinder<UAnimMontage> ReloadTemp(TEXT("/Script/Engine.AnimMontage'/Game/LEH/Animations/AM_Reload.AM_Reload'"));
	if (ReloadTemp.Succeeded())
	{
		ReloadMontage = ReloadTemp.Object;
	}

	ConstructorHelpers::FObjectFinder<UAnimMontage> FireTemp(TEXT("/Script/Engine.AnimMontage'/Game/LEH/Animations/AM_Fire.AM_Fire'"));
	if (FireTemp.Succeeded())
	{
		FireMontage = FireTemp.Object;
	}

	ConstructorHelpers::FObjectFinder<UAnimMontage> SaluteTemp(TEXT("/Script/Engine.AnimMontage'/Game/LEH/Animations/AM_Salute.AM_Salute'"));
	if (SaluteTemp.Succeeded())
	{
		SaluteMontage = SaluteTemp.Object;
	}
}

void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	APawn* Owner = TryGetPawnOwner();
	if (!Owner) return;

	// FSM 플레이어 스테이트 갱신
	UPlayerFSM* PlayerFSM = Owner->FindComponentByClass<UPlayerFSM>();
	if (PlayerFSM)
	{
		_PlayerState = PlayerFSM->GetPlayerState();
	}
	
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(Owner);
	if (PlayerCharacter)
	{
		FVector velocity = PlayerCharacter->GetVelocity();
		FVector forward = PlayerCharacter->GetActorForwardVector();
		FVector right = PlayerCharacter->GetActorRightVector();

		// 속도 갱신 
		Speed = FVector::DotProduct(velocity, forward);

		// 방향 갱신
		Direction = FVector::DotProduct(velocity, right);

		// Aiming
		bIsAiming = PlayerCharacter->bIsPlayerAiming;
		AimingLocation = PlayerCharacter->GetCameraAim();

		// Current weapon
		CurrentWeapon = PlayerCharacter->GetCurrentWeaponIdx();
	}
}

void UPlayerAnimInstance::PlayReloadAnimation()
{
	Montage_Play(ReloadMontage);
}

void UPlayerAnimInstance::PlayFireAnimation()
{
	Montage_Play(FireMontage);
}

void UPlayerAnimInstance::PlaySaluteAnimation()
{
	StopAllMontages(0.1);
	Montage_Play(SaluteMontage);
}

void UPlayerAnimInstance::StopCurrentAnimation()
{
	UAnimMontage* CurrentMontage = GetCurrentActiveMontage();
	if (CurrentMontage)
	{
		Montage_Stop(0.25, CurrentMontage);
	}
}
