// Fill out your copyright notice in the Description page of Project Settings.


#include "LEH/SuperPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "LEH/PlayerCharacter.h"
#include "LEH/PlayerFSM.h"
#include "OSC/Weapon/WeaponComponent.h"

void ASuperPlayerController::BeginPlay()
{
	Super::BeginPlay();

	UEnhancedInputLocalPlayerSubsystem* Subsystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (Subsystem)
	{
		Subsystem->AddMappingContext(IMC_Player, 0);
		Subsystem->AddMappingContext(IMC_Weapon, 0);
	}

	// Get owning Player character
	PlayerCharacter = Cast<APlayerCharacter>(GetPawn());
	if (PlayerCharacter)
	{
		PlayerCharacter->OnZoomInCompleted.AddDynamic(this, &ASuperPlayerController::CrossHairWidgetOn);
	}

	// Get owning player FSM
	PlayerFSMComp = GetPawn()->FindComponentByClass<UPlayerFSM>();

	// Get owning player weapon component
	WeaponComp = GetPawn()->FindComponentByClass<UWeaponComponent>();

	// CrossHair widget
	if (CrossHairWidgetClass)
	{
		CrossHairWidget = CreateWidget<UUserWidget>(this, CrossHairWidgetClass);
	}
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
		// Look
		EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASuperPlayerController::Look);
		// Sprint
		EnhancedInput->BindAction(SprintAction, ETriggerEvent::Started, this, &ASuperPlayerController::SprintStart);
		EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &ASuperPlayerController::SprintEnd);

		// Aiming
		EnhancedInput->BindAction(AimingAction, ETriggerEvent::Started, this, &ASuperPlayerController::AimingStart);
		EnhancedInput->BindAction(AimingAction, ETriggerEvent::Completed, this, &ASuperPlayerController::AimingEnd);
		
		// Fire
		EnhancedInput->BindAction(FireAction, ETriggerEvent::Started, this, &ASuperPlayerController::FireStart);
		EnhancedInput->BindAction(FireAction, ETriggerEvent::Completed, this, &ASuperPlayerController::FireEnd);

		// Reload
		EnhancedInput->BindAction(ReloadAction, ETriggerEvent::Started, this, &ASuperPlayerController::Reload);

		// Dive
		EnhancedInput->BindAction(DiveAction, ETriggerEvent::Started, this, &ASuperPlayerController::Dive);

		// Salute
		EnhancedInput->BindAction(SaluteAction, ETriggerEvent::Started, this, &ASuperPlayerController::Salute);
		
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
	if (IsValid(PlayerFSMComp))
	{
		PlayerFSMComp->SetPlayerState(EPlayerState::Move);
	}
}

void ASuperPlayerController::MoveEnd(const FInputActionValue& Value)
{
	if (IsValid(PlayerFSMComp))
	{
		PlayerFSMComp->SetPlayerState(EPlayerState::Idle);
	}
}

void ASuperPlayerController::Look(const FInputActionValue& Value)
{
	FVector2D v = Value.Get<FVector2D>();
	
	GetPawn()->AddControllerYawInput(v.X);
	GetPawn()->AddControllerPitchInput(v.Y);
}

void ASuperPlayerController::SprintStart(const FInputActionValue& Value)
{
	if (bIsAiming || bIsReloading)
	{
		return;
	}
	
	PlayerCharacter->GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void ASuperPlayerController::SprintEnd(const FInputActionValue& Value)
{
	if (bIsAiming)
	{
		return;
	}
	
	PlayerCharacter->GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ASuperPlayerController::AimingStart(const FInputActionValue& Value)
{
	if (bIsReloading || bIsAiming)
	{
		return;
	}
	
	bIsAiming = true;
	PlayerCharacter->bIsPlayerAiming = true;
	PlayerCharacter->GetCharacterMovement()->MaxWalkSpeed = AimingSpeed;
		
	WeaponComp->StartAiming();
		
	// Actor rotation
	PlayerCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
	PlayerCharacter->GetCharacterMovement()->bUseControllerDesiredRotation = true;
	PlayerCharacter->GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);
	PlayerCharacter->bUseControllerRotationYaw = false;

	// Set FOV
	PlayerCharacter->StartZoom(true);
}

void ASuperPlayerController::AimingEnd(const FInputActionValue& Value)
{
	//if (bIsReloading || !bIsAiming)
	//{
	//	return;
	//}

	bIsAiming = false;
	PlayerCharacter->bIsPlayerAiming = false;
	PlayerCharacter->GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		
	WeaponComp->StopAiming();

	// Actor rotation
	PlayerCharacter->GetCharacterMovement()->bUseControllerDesiredRotation = false;
	PlayerCharacter->GetCharacterMovement()->bOrientRotationToMovement = true;

	// Set FOV
	PlayerCharacter->StartZoom(false);

	// UI
	CrossHairWidget->RemoveFromParent();
}

void ASuperPlayerController::CrossHairWidgetOn()
{
	CrossHairWidget->AddToViewport();
}

void ASuperPlayerController::FireStart(const FInputActionValue& Value)
{
	if (bIsAiming)
	{
		if (IsValid(WeaponComp))
		{
			PlayerCharacter->PlayFireMontage();
			WeaponComp->StartFire();
		}
	}
}

void ASuperPlayerController::FireEnd(const FInputActionValue& Value)
{
	if (IsValid(WeaponComp))
	{
		WeaponComp->StopFire();
	}
}

void ASuperPlayerController::Reload(const FInputActionValue& Value)
{
	if (!bIsReloading)
	{
		bIsReloading = true;
		PlayerCharacter->PlayReloadMontage();

		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([&]
		{
			bIsReloading = false;
			
		}), ReloadingTime, false);
	}
	
	if (IsValid(WeaponComp))
	{
		WeaponComp->Reload();
	}
}

void ASuperPlayerController::Dive(const FInputActionValue& Value)
{
	PlayerFSMComp->SetPlayerState(EPlayerState::Prone);
	
	
}

void ASuperPlayerController::Salute(const FInputActionValue& Value)
{
	PlayerCharacter->PlaySaluteMontage();
}
