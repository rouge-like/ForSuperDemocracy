// Fill out your copyright notice in the Description page of Project Settings.


#include "LEH/PlayerFSM.h"

#include "LEH/PlayerCharacter.h"

// Sets default values for this component's properties
UPlayerFSM::UPlayerFSM()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPlayerFSM::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void UPlayerFSM::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	GEngine->AddOnScreenDebugMessage(0, 1, FColor::Cyan, UEnum::GetValueAsString(mState));
	
	switch (mState)
	{
	case EPlayerState::Idle:
		IdleState();
		break;
	case EPlayerState::Move:
		MoveState();
		break;
	case EPlayerState::Prone:
		ProneState();
		break;
	case EPlayerState::Damage:
		DamageState();
		break;
	case EPlayerState::Die:
		DieState();
		break;
	case EPlayerState::Salute:
		SaluteState();
		break;
	}
}

void UPlayerFSM::IdleState()
{
}

void UPlayerFSM::MoveState()
{
}

void UPlayerFSM::ProneState()
{
}

void UPlayerFSM::DamageState()
{
}

void UPlayerFSM::DieState()
{
}

void UPlayerFSM::SaluteState()
{
}



