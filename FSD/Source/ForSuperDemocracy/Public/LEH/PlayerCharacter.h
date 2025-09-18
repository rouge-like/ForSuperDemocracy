// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "OSC/Weapon/WeaponBase.h"

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

public:
	void OnConstruction(const FTransform& Transform) override;
	// Components
	UPROPERTY(EditAnywhere)
	class USpringArmComponent* SpringArm;
	
	UPROPERTY(EditAnywhere)
	class UCameraComponent* Camera;

	// Rifle
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UChildActorComponent* ChildActor;
	// Grenade
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UChildActorComponent* ChildActor1;
    // Staratagem
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UChildActorComponent* ChildActor2;

	UPROPERTY(EditAnywhere)
	class UPlayerFSM* FSMComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UWeaponComponent* WeaponComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UHealthComponent* HealthComp;

public:
	// Damage
	float DamageTime = 1.f;
	FTimerHandle DamageTimerHandle;
	
	UFUNCTION()
	void OnDamaged(float Damage, AActor* DamageCauser, AController* EventInstigator, TSubclassOf<UDamageType> DamageType);

public:
	UFUNCTION()
	void OnDeath(AActor* Victim);
	
protected:
	// FOV lerp
	UPROPERTY(EditDefaultsOnly, Category=FOVLerp)
	float MaxFOV = 90.f;

	UPROPERTY(EditDefaultsOnly, Category=FOVLerp)
	float MinFOV = 40.f;

	UPROPERTY(EditAnywhere, Category=FOVLerp)
	float LerpSpeed = 3.5f;
	
	bool bIsZooming = false;
	
	float ZoomStartFOV;
	float ZoomTargetFOV;
	
	float CurrentLerpAlpha1 = 0.f;

public:
	UPROPERTY(BlueprintAssignable)
	FOnZoomInCompleted OnZoomInCompleted;
	
	void StartZoom(bool IsAiming);

public:
	// Aiming rotation
	bool bIsPlayerAiming = false;

	FRotator GetCameraAim();

public:
	// Camera Prone
	UPROPERTY(EditDefaultsOnly, Category=CameraProne)
	float MaxHeight = 0.f;

	UPROPERTY(EditDefaultsOnly, Category=CameraProne)
	float MinHeight = -100.f;

	UPROPERTY(EditAnywhere, Category=CameraProne)
	float CameraLerpSpeed = 1.5f;
	
	bool bIsCameraProning = false;
	bool bEasingFlag = true;
	
	float StartZ;
	float TargetZ;

	float CurrentLerpAlpha2 = 0.f;
	
	void StartCameraProne(bool IsProning);
	
	float easeOutCubic(float x);
	float easeInCubic(float x);

public:
	// Fire
	UFUNCTION()
	void OnWeaponFired(AWeaponBase* Weapon);
	
public:
	// Montage
	void PlayReloadMontage();
	void PlayFireMontage();
	void PlaySaluteMontage();
	void StopSaluteMontage();
	
	bool bIsPlayerSalute = false;
};
