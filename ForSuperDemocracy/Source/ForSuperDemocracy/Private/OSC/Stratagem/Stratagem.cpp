// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/Stratagem/Stratagem.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
AStratagem::AStratagem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
}

// Called when the game starts or when spawned
void AStratagem::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AStratagem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

