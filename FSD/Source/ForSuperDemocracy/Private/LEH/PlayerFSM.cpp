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

	GEngine->AddOnScreenDebugMessage(0, 1, FColor::Cyan, UEnum::GetValueAsString(_State));
	
}

EPlayerState UPlayerFSM::GetPlayerState()
{
	return _State;
}

void UPlayerFSM::SetPlayerState(EPlayerState NewState)
{
	if (_State == NewState)
	{
		return;
	}
	
	// 바꾸기 전 상태 저장
	if (NewState == EPlayerState::Damage && _State == EPlayerState::Damage)
	{
		return;
	}

	PreviousState = _State;
	_State = NewState;

	//UE_LOG(LogTemp, Warning, TEXT("Prev : %s, Cur : %s"), *UEnum::GetValueAsString(PreviousState),*UEnum::GetValueAsString(_State));
}

EPlayerState UPlayerFSM::GetPreviousPlayerState()
{
	return PreviousState;
}



