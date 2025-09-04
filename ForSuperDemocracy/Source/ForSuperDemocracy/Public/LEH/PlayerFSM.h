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
	Prone, // 엎드리기, 다이빙
	Damage,
	Die,
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

public:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = FSM)
	EPlayerState mState = EPlayerState::Idle;
	
	// 대기 상태
	void IdleState();
	// 이동 상태
	void MoveState();
	// 엎드리기, 다이빙
	void ProneState();
	// 피격 상태
	void DamageState();
	// 죽음 상태
	void DieState();
	// 경례 상태
	void SaluteState();
	
	// 플레이어 움직임, 플레이어 애니메이션, TPS 카메라
};
