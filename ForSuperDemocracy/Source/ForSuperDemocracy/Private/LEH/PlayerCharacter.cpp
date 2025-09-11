// Fill out your copyright notice in the Description page of Project Settings.


#include "LEH/PlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "LEH/PlayerFSM.h"
#include "OSC/Weapon/WeaponComponent.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	
	Camera =CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	ChildActor = CreateDefaultSubobject<UChildActorComponent>(TEXT("ChildActor"));
	ChildActor->SetupAttachment(GetMesh());
	
	FSMComp = CreateDefaultSubobject<UPlayerFSM>(TEXT("FSMComponent"));
	WeaponComp = CreateDefaultSubobject<UWeaponComponent>(TEXT("WeaponComponent"));
	
	ConstructorHelpers::FObjectFinder<USkeletalMesh> BodyMeshTemp(TEXT("/Script/Engine.SkeletalMesh'/Game/PROJECTS/HELLDIVERS_2/CHARACTERS/PLAYER/B-01_TACTICAL/fix_v2/SKM_B-01_v1_BRAWNY_SIMPLE.SKM_B-01_v1_BRAWNY_SIMPLE'"));
	if (BodyMeshTemp.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(BodyMeshTemp.Object);
	}
	
	GetMesh()->SetRelativeLocation(FVector(0.000000,0.000000,-83.103748));
	GetMesh()->SetRelativeRotation(FRotator(0.000000,-89.999999,0.000000));
	
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Weapon 부착
	FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);
	ChildActor->AttachToComponent(GetMesh(), AttachRules, FName("hand_r_socket"));
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsZooming)
	{
		CurrentLerpAlpha = FMath::Clamp(CurrentLerpAlpha+DeltaTime*LerpSpeed, 0.0f, 1.0f);

		float NewFOV = FMath::Lerp(ZoomStartFOV, ZoomTargetFOV, CurrentLerpAlpha);
		Camera->SetFieldOfView(NewFOV);

		if (FMath::IsNearlyEqual(CurrentLerpAlpha, 1.f, 0.001f))
		{
			if (ZoomTargetFOV == MinFOV)
			{
				OnZoomInCompleted.Broadcast();
			}
			
			CurrentLerpAlpha = 0.f;
			bIsZooming = false;
		}
	}
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void APlayerCharacter::StartZoom(bool IsAiming)
{
	bIsZooming = true;

	if (IsAiming)
	{
		ZoomStartFOV = Camera->FieldOfView; // 현재 FOV에서 시작
		ZoomTargetFOV = MinFOV;
		CurrentLerpAlpha = 0.0f;
	}
	else
	{
		ZoomStartFOV = Camera->FieldOfView;
		ZoomTargetFOV = MaxFOV;
		CurrentLerpAlpha = 0.0f;
	}
}

FRotator APlayerCharacter::GetCameraAim()
{
	FVector Location;
	FRotator Rotation;
	
	if (GetController())
	{
		GetController()->GetPlayerViewPoint(Location, Rotation);
		//UE_LOG(LogTemp, Warning, TEXT("%f, %f, %f"), Rotation.Roll, Rotation.Pitch, Rotation.Yaw);
		
		return Rotation;
	}

	// 컨트롤러가 없는 경우(예: AI에 의해 제어될 때)를 위한 대체 동작
	return GetBaseAimRotation();
}


