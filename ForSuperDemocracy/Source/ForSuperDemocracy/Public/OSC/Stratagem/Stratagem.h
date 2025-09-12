// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OSC/Weapon/ProjectileBase.h"
#include "Stratagem.generated.h"

class USphereComponent;
class USkeletalMeshComponent;
class UProjectileMovementComponent;
class UAnimSequenceBase;
class UNiagaraSystem;

UCLASS()
class FORSUPERDEMOCRACY_API AStratagem : public AProjectileBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AStratagem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// 딜레이 시간 후 액션 및 자멸
	UPROPERTY(EditDefaultsOnly, Category="Stratagem")
	float DelayTime = 2.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stratagem|Animation")
	TObjectPtr<UAnimSequenceBase> FixingAnim;
	
	UPROPERTY(EditDefaultsOnly, Category = "Stratagem|Animation")
	TObjectPtr<UAnimSequenceBase> FixedAnim;

	UPROPERTY(EditDefaultsOnly, Category="Stratagem|VFX")
	TObjectPtr<UNiagaraSystem> LightVFX;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
