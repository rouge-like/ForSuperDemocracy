// Fill out your copyright notice in the Description page of Project Settings.


#include "LEH/PlayerAnimInstance.h"

#include "LEH/PlayerCharacter.h"
#include "LEH/PlayerFSM.h"
#include "LEH/SuperPlayerController.h"


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
	
	// 속도 갱신 
	FVector Velocity = Owner->GetVelocity();
	FVector Foward = Owner->GetActorForwardVector();
	Speed = FVector::DotProduct(Velocity, Foward);
	//Speed = Velocity.Size();

	// 방향 갱신
	Direction = CalculateDirection(Velocity, Owner->GetActorRotation());
	UE_LOG(LogTemp, Warning, TEXT("%f"), Speed);
	UE_LOG(LogTemp, Warning, TEXT("%f"), Direction);

	// Aiming
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(Owner);
	if (PlayerCharacter)
	{
		bIsAiming = PlayerCharacter->bIsPlayerAiming;
		AimingLocation = PlayerCharacter->GetCameraAim();
	}
		
}
