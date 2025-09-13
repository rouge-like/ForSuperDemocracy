// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "SuperPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class FORSUPERDEMOCRACY_API ASuperPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	
	virtual void SetupInputComponent() override;

public:
	class APlayerCharacter* PlayerCharacter;
	class UPlayerFSM* PlayerFSMComp;
	class UWeaponComponent* WeaponComp;
	
protected:
	/////////////// IMC_Player
	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputMappingContext* IMC_Player;

	// Move
	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* MoveAction;

	void Move(const FInputActionValue& Value);
	void MoveStart(const FInputActionValue& Value);
	void MoveEnd(const FInputActionValue& Value);

	// Look
	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* LookAction;

	void Look(const FInputActionValue& Value);

	// Sprint
	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* SprintAction;
	
	void SprintStart(const FInputActionValue& Value);
	void SprintEnd(const FInputActionValue& Value);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Speed)
	float SprintSpeed = 900.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Speed)
	float WalkSpeed = 600.f;

protected:
	/////////////// IMC_Weapon
	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputMappingContext* IMC_Weapon;

	// Aiming
	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* AimingAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Speed)
	float AimingSpeed = 300.f;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = UI)
	TSubclassOf<class UUserWidget> CrossHairWidgetClass;

	UPROPERTY()
	UUserWidget* CrossHairWidget;
	
	bool bIsAiming = false;
	
	void AimingStart(const FInputActionValue& Value);
	void AimingEnd(const FInputActionValue& Value);
	
	UFUNCTION()
	void CrossHairWidgetOn();
	
	// Fire
	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* FireAction;

	void FireStart(const FInputActionValue& Value);
	void FireEnd(const FInputActionValue& Value);
	
	// Reload
	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* ReloadAction;

	bool bIsReloading = false;

	UPROPERTY(EditAnywhere, Category = Reload)
	float ReloadingTime = 2.2f;
	
	void Reload(const FInputActionValue& Value);
	
	// Dive
	UPROPERTY(EditDefaultsOnly, Category = Input)
    class UInputAction* DiveAction;
	
	void Dive(const FInputActionValue& Value);
};
