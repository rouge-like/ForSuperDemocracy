// Fill out your copyright notice in the Description page of Project Settings.


#include "LEH/HellPod.h"

#include "NiagaraFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "LEH/PlayerCharacter.h"


// Sets default values
AHellPod::AHellPod()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Collision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Collision"));
	SetRootComponent(Collision);
	
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(GetRootComponent());
	
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SprintArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	
	ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshTemp(TEXT("/Script/Engine.SkeletalMesh'/Game/PROJECTS/HELLDIVERS_2/PROPS/GAMEPLAY/HELLPOD_VARIANTS/HELLPOD_PLAYER/FIX/SK_HELLPOD_PLAYER_fix.SK_HELLPOD_PLAYER_fix'"));
	if (MeshTemp.Succeeded())
	{
		Mesh->SetSkeletalMesh(MeshTemp.Object);
	}

	// Camera settings
	SpringArm->SetRelativeRotation(FRotator(-90.000000,0.000000,0.000000));
	SpringArm->TargetArmLength = 600.f;
}

// Called when the game starts or when spawned
void AHellPod::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PlayerController) return;
	
	// 뷰 타겟 전환 (부드럽게 블렌드하면서)
	PlayerController->SetViewTargetWithBlend(this, 1.0f); // 1.0초 동안 부드럽게 전환

	// Overlap
	Collision->OnComponentBeginOverlap.AddDynamic(this, &AHellPod::OnBeginOverlap);
}

void AHellPod::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([&]
	{
		bIsFalling = false;

		Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// 플레이어를 가져온다,
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC)
		{
			APlayerCharacter* CurrentPawn = Cast<APlayerCharacter>(PC->GetPawn());
			if (CurrentPawn)
			{
				CurrentPawn->RespawnPlayer(GetActorLocation());
			}

			PC->SetViewTargetWithBlend(CurrentPawn, 1.f);
		}
		
	}), DigInTime, false);

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		Smoke,
		GetActorLocation(),
		GetActorRotation()
	);
}

// Called every frame
void AHellPod::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsFalling)
	{
		FVector CurrentLocation = GetActorLocation();
		float NewZ = CurrentLocation.Z + Speed *DeltaTime * -9.8f;
	
		SetActorLocation(FVector(CurrentLocation.X, CurrentLocation.Y, NewZ));
	}

	
}

