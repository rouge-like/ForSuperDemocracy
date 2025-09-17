// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerFSM.generated.h"

UENUM(BlueprintType)
enum class EPlayerState:uint8
{
	Idle, 
	Move, // 걷기, 달리기
	Prone, // 엎드리기
	Dive, // 다이빙해서 엎드리기
	Damage,
	Dead,
	Salute, // 경례
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FORSUPERDEMOCRACY_API UPlayerFSM : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlayerFSM();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = FSM)
	EPlayerState _State = EPlayerState::Idle;
	
public:
	// State getter, setter
	EPlayerState GetPlayerState();
	void SetPlayerState(EPlayerState NewState);

	EPlayerState PreviousState;
	EPlayerState GetPreviousPlayerState();
	
};
