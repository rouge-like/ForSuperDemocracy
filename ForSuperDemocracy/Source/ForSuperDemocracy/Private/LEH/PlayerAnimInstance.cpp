// Fill out your copyright notice in the Description page of Project Settings.


#include "LEH/PlayerAnimInstance.h"

void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	APawn* Owner = TryGetPawnOwner();
	if (!Owner) return;

	FVector Velocity = Owner->GetVelocity();
	Speed = Velocity.Size();
	
	//FVector Velocity = TryGetPawnOwner()->GetVelocity();
	//Speed = Velocity.Size();
}
