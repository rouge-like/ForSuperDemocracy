// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/Weapon/Grenade.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

// Sets default values
AGrenade::AGrenade()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SphereComp = CreateDefaultSubobject<USphereComponent>("CapsuleComp");
	SetRootComponent(SphereComp);

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>("MeshComp");
	MeshComp->SetupAttachment(SphereComp);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
}

// Called when the game starts or when spawned
void AGrenade::BeginPlay()
{
    Super::BeginPlay();
    // 2초(또는 FuseTime) 후 폭발 FX 스폰 및 자멸
    if (UWorld* World = GetWorld())
    {
        FTimerHandle TimerHandle_Fuse;
        World->GetTimerManager().SetTimer(
            TimerHandle_Fuse,
            this,
            &AGrenade::Explode,
            FMath::Max(0.01f, FuseTime),
            false
        );
    }
}

// Called every frame
void AGrenade::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}

void AGrenade::Explode()
{
    // 폭발 FX 스폰 (Cascade Particle System)
    if (ExplosionFX)
    {
        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            ExplosionFX,
            GetActorLocation(),
            GetActorRotation()
        );
    }

    Destroy();
}

