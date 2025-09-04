// Fill out your copyright notice in the Description page of Project Settings.


#include "LEH/SuperPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "LEH/PlayerCharacter.h"
#include "LEH/PlayerFSM.h"

void ASuperPlayerController::BeginPlay()
{
	Super::BeginPlay();

	UEnhancedInputLocalPlayerSubsystem* Subsystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (Subsystem)
	{
		Subsystem->AddMappingContext(IMC_Player, 0);
	}

	PlayerFSM = GetPawn()->FindComponentByClass<UPlayerFSM>();
	IsValid(PlayerFSM);
}

void ASuperPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
	if (EnhancedInput)
	{
		// Move
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASuperPlayerController::Move);
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Started, this, &ASuperPlayerController::MoveStart);
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Completed, this, &ASuperPlayerController::MoveEnd);
		
		// View Control
		EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASuperPlayerController::Look);
		// Sprint
		EnhancedInput->BindAction(SprintAction, ETriggerEvent::Started, this, &ASuperPlayerController::SprintStart);
		EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &ASuperPlayerController::SprintEnd);
	}
}

void ASuperPlayerController::Move(const FInputActionValue& Value)
{
	FVector2D Input = Value.Get<FVector2D>();
	
	// 1. 컨트롤러 회전 (카메라 기준)
	const FRotator ControlRot = GetControlRotation();

	// 2. Yaw만 남기고 Pitch, Roll 제거
	const FRotator YawRot(0.f, ControlRot.Yaw, 0.f);

	// 3. Forward / Right 방향 계산 (Yaw 기준)
	const FVector Forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X); // 앞뒤
	const FVector Right   = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y); // 좌우

	// 4. 입력값에 따라 이동
	GetPawn()->AddMovementInput(Right,   Input.Y); // 좌우
	GetPawn()->AddMovementInput(Forward, Input.X); // 앞뒤
}

void ASuperPlayerController::MoveStart(const FInputActionValue& Value)
{
	PlayerFSM->mState = EPlayerState::Move;
}

void ASuperPlayerController::MoveEnd(const FInputActionValue& Value)
{
	PlayerFSM->mState = EPlayerState::Idle;
}

void ASuperPlayerController::Look(const FInputActionValue& Value)
{
	FVector2D v = Value.Get<FVector2D>();
	
	GetPawn()->AddControllerYawInput(v.X);
	GetPawn()->AddControllerPitchInput(v.Y);
}

void ASuperPlayerController::SprintStart(const FInputActionValue& Value)
{
	ACharacter* PlayerCharacter = Cast<ACharacter>(GetPawn());
	if (PlayerCharacter)
	{
		PlayerCharacter->GetCharacterMovement()->MaxWalkSpeed = PlayerCharacter->GetCharacterMovement()->MaxWalkSpeed + SpeedIncreaseValue;
	}
}

void ASuperPlayerController::SprintEnd(const FInputActionValue& Value)
{
	ACharacter* PlayerCharacter = Cast<ACharacter>(GetPawn());
	if (PlayerCharacter)
	{
		PlayerCharacter->GetCharacterMovement()->MaxWalkSpeed = PlayerCharacter->GetCharacterMovement()->MaxWalkSpeed - SpeedIncreaseValue;
	}
}
