// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "PlayerCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnZoomInCompleted);

UCLASS()
class FORSUPERDEMOCRACY_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	void OnConstruction(const FTransform& Transform) override;
	// Components
	UPROPERTY(EditAnywhere)
	class USpringArmComponent* SpringArm;
	
	UPROPERTY(EditAnywhere)
	class UCameraComponent* Camera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UChildActorComponent* ChildActor;

	UPROPERTY(EditAnywhere)
	class UPlayerFSM* FSMComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UWeaponComponent* WeaponComp;

protected:
	// FOV lerp
	UPROPERTY(EditDefaultsOnly, Category=FOV)
	float MaxFOV = 90.f;

	UPROPERTY(EditDefaultsOnly, Category=FOV)
	float MinFOV = 70.f;

	UPROPERTY(EditAnywhere, Category=FOV)
	float LerpSpeed = 5.f;
	
	bool bIsZooming = false;
	
	float ZoomStartFOV;
	float ZoomTargetFOV;
	
	float CurrentLerpAlpha = 0.f;

public:
	UPROPERTY(BlueprintAssignable)
	FOnZoomInCompleted OnZoomInCompleted;
	
	void StartZoom(bool IsAiming);

public:
	// Aiming rotation
	bool bIsPlayerAiming = false;

	FRotator GetCameraAim();

public:
	// Montage
	void PlayReloadMontage();
	void PlayFireMontage();
	void PlaySaluteMontage();
	void StopSaluteMontage();
	
	bool bIsPlayerSalute = false;
};
