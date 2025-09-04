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
	class UPlayerFSM* PlayerFSM;
	
protected:
	// Move
	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputMappingContext* IMC_Player;

	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* LookAction;
	
	UPROPERTY(EditDefaultsOnly, Category = Input)
	class UInputAction* SprintAction;

	void Move(const FInputActionValue& Value);
	void MoveStart(const FInputActionValue& Value);
	void MoveEnd(const FInputActionValue& Value);
	
	void Look(const FInputActionValue& Value);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	float SpeedIncreaseValue = 300.f;
	
	void SprintStart(const FInputActionValue& Value);
	void SprintEnd(const FInputActionValue& Value);

	
};
