// Fill out your copyright notice in the Description page of Project Settings.


#include "LEH/PlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "LEH/PlayerAnimInstance.h"
#include "LEH/PlayerFSM.h"
#include "LEH/SuperPlayerController.h"
#include "OSC/HealthComponent.h"
#include "OSC/Weapon/WeaponComponent.h"
#include "OSC/Weapon/WeaponBase.h"

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

	ChildActor1 = CreateDefaultSubobject<UChildActorComponent>(TEXT("ChildActor1"));
	ChildActor1->SetupAttachment(GetMesh());

	ChildActor2 = CreateDefaultSubobject<UChildActorComponent>(TEXT("ChildActor2"));
	ChildActor2->SetupAttachment(GetMesh());
	
	FSMComp = CreateDefaultSubobject<UPlayerFSM>(TEXT("FSMComponent"));
	WeaponComp = CreateDefaultSubobject<UWeaponComponent>(TEXT("WeaponComponent"));
	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	
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
    // FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);
    // ChildActor->AttachToComponent(GetMesh(), AttachRules, FName("hand_r_socket"));

    // Camera kick 초기화
    if (IsValid(SpringArm))
    {
        DefaultArmLength = SpringArm->TargetArmLength;
    }

	WeaponComp->OnWeaponFired.AddDynamic(this, &APlayerCharacter::ApplyCameraKick);
	if (HealthComp)
	{
		HealthComp->OnDamaged.AddDynamic(this, &APlayerCharacter::OnDamaged);
		HealthComp->OnDeath.AddDynamic(this, &APlayerCharacter::OnDeath);
	}

	if (WeaponComp)
	{
		WeaponComp->OnWeaponFired.AddDynamic(this, &APlayerCharacter::OnWeaponFired);
	}
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsZooming)
	{
		CurrentLerpAlpha1 = FMath::Clamp(CurrentLerpAlpha1+DeltaTime*LerpSpeed, 0.0f, 1.0f);

		float NewFOV = FMath::Lerp(ZoomStartFOV, ZoomTargetFOV, CurrentLerpAlpha1);
		Camera->SetFieldOfView(NewFOV);

		if (FMath::IsNearlyEqual(CurrentLerpAlpha1, 1.f, 0.001f))
		{
			if (ZoomTargetFOV == MinFOV)
			{
				OnZoomInCompleted.Broadcast();
			}
			
			CurrentLerpAlpha1 = 0.f;
			bIsZooming = false;
		}
	}

    if (bIsPlayerSalute)
    {
        if (FSMComp->GetPlayerState() != EPlayerState::Salute)
        {
            StopSaluteMontage();
        }
    }

    // Camera kickback 복구 처리
    if (IsValid(SpringArm))
    {
        if (CurrentCameraKick > 0.f)
        {
            const float Recover = CameraKickReturnSpeed * DeltaTime;
            const float Step = FMath::Min(CurrentCameraKick, Recover);
            CurrentCameraKick -= Step;
        }
        // 기본 길이 + 현재 킥백 반영
        SpringArm->TargetArmLength = DefaultArmLength + CurrentCameraKick;
    }
	// Camera height Prone
	if (bIsCameraProning)
	{
		CurrentLerpAlpha2 = FMath::Clamp(CurrentLerpAlpha2+DeltaTime*CameraLerpSpeed, 0.0f, 1.0f);
		float EasedAlpha = bEasingFlag ? easeOutCubic(CurrentLerpAlpha2) : easeInCubic(CurrentLerpAlpha2);

		float NewZLoc = FMath::Lerp(StartZ, TargetZ, EasedAlpha);

		FVector CurrentLocation = SpringArm->GetRelativeLocation();
		FVector NewLocation = FVector(CurrentLocation.X, CurrentLocation.Y, NewZLoc);
		SpringArm->SetRelativeLocation(NewLocation);

		if (FMath::IsNearlyEqual(CurrentLerpAlpha2, 1.f, 0.001f))
		{
			CurrentLerpAlpha2 = 0.f;
			
			bIsCameraProning = false;
		}
		
	}

	if (bIsPlayerSalute)
	{
		if (FSMComp->GetPlayerState() != EPlayerState::Salute)
		{
			StopSaluteMontage();
		}
	}
}

void APlayerCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Rifle settings
	FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);
	ChildActor->AttachToComponent(GetMesh(), AttachRules, FName("hand_r_socket"));
	ChildActor->CreateChildActor();
	ChildActor->GetChildActor()->SetOwner(this);
	ChildActor->GetChildActor()->SetInstigator(this);
	ChildActor->SetRelativeLocation(FVector(-0.5f, 8.5f, -1.0f));
	ChildActor->SetRelativeRotation(FRotator(0, 0, 0));

	// Grenade settings
	ChildActor1->AttachToComponent(GetMesh(), AttachRules, FName("hand_r_socket"));

	// Staratagem settings
	ChildActor2->AttachToComponent(GetMesh(), AttachRules, FName("hand_r_socket"));
}

void APlayerCharacter::OnDamaged(float Damage, AActor* DamageCauser, AController* EventInstigator,
	TSubclassOf<UDamageType> DamageType)
{
	// 뭘 넣어야해?
	// 데미지 받았을 때 뭘 할거야?
	// 1. 피격 애니메이션 재생
	if(FSMComp->GetPlayerState() == EPlayerState::Damage || FSMComp->GetPlayerState() == EPlayerState::Dead)
	{
		return;
	}
	
	FSMComp->SetPlayerState(EPlayerState::Damage);
	
	GetWorldTimerManager().SetTimer(DamageTimerHandle, FTimerDelegate::CreateLambda([&]
	{
		FSMComp->SetPlayerState(FSMComp->GetPreviousPlayerState());
		
	}), DamageTime, false);
}

void APlayerCharacter::OnDeath(AActor* Victim)
{
	GetWorldTimerManager().ClearTimer(DamageTimerHandle);
	FSMComp->SetPlayerState(EPlayerState::Dead);
}


void APlayerCharacter::StartZoom(bool IsAiming)
{
	bIsZooming = true;

	if (IsAiming)
	{
		ZoomStartFOV = Camera->FieldOfView; // 현재 FOV에서 시작
		ZoomTargetFOV = MinFOV;
		CurrentLerpAlpha1 = 0.0f;
	}
	else
	{
		ZoomStartFOV = Camera->FieldOfView;
		ZoomTargetFOV = MaxFOV;
		CurrentLerpAlpha1 = 0.0f;
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

void APlayerCharacter::StartCameraProne(bool IsProning)
{
	bIsCameraProning = true;

	FTimerHandle TimerHandle;

	// prone <-> idle/move 사이 움직일 텀 필요
	GetCharacterMovement()->SetMovementMode(MOVE_None);
	GetWorldTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([&]
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}), 0.1f, false);
	
	if (IsProning)
	{
		StartZ = SpringArm->GetRelativeLocation().Z;
		TargetZ = MinHeight;
		CurrentLerpAlpha2 = 0.0f;

		bEasingFlag = true;
	}
	else
	{
		StartZ = SpringArm->GetRelativeLocation().Z;
		TargetZ = MaxHeight;
		CurrentLerpAlpha2 = 0.0f;

		bEasingFlag = false;
	}
}

float APlayerCharacter::easeOutCubic(float x)
{
	return 1 - FMath::Pow(1 - x, 3);
}

float APlayerCharacter::easeInCubic(float x)
{
	return x * x * x;
}

void APlayerCharacter::OnWeaponFired(AWeaponBase* Weapon)
{
	PlayFireMontage();
}

void APlayerCharacter::PlayReloadMontage()
{
	// Aim 중 reload 시 aim해제
	ASuperPlayerController* PC = Cast<ASuperPlayerController>(GetController());
	if (PC && PC->bIsAiming)
	{
		PC->AimingEnd(0);
	}
	
	auto anim = Cast<UPlayerAnimInstance>(GetMesh()->GetAnimInstance());
	if (anim)
	{
		anim->PlayReloadAnimation();
	}
}

void APlayerCharacter::PlayFireMontage()
{
	auto anim = Cast<UPlayerAnimInstance>(GetMesh()->GetAnimInstance());
	if (anim)
	{
		anim->PlayFireAnimation();
	}
}

void APlayerCharacter::PlaySaluteMontage()
{
	if (bIsPlayerSalute)
	{
		return;
	}
	
	auto anim = Cast<UPlayerAnimInstance>(GetMesh()->GetAnimInstance());
	if (anim)
	{
		bIsPlayerSalute = true;
		
		ChildActor->SetVisibility(false);
		anim->PlaySaluteAnimation();
	}
}

void APlayerCharacter::StopSaluteMontage()
{
	ChildActor->SetVisibility(true);
	
	auto anim = Cast<UPlayerAnimInstance>(GetMesh()->GetAnimInstance());
	if (anim)
	{
		bIsPlayerSalute = false;

		anim->StopCurrentAnimation();
	}
}


void APlayerCharacter::ApplyCameraKick(AWeaponBase* Weapon)
{
    if (!IsValid(SpringArm)) return;
	
    CurrentCameraKick += 5.0f;
}
