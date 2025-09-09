// Fill out your copyright notice in the Description page of Project Settings.


// Sets default values
#include "Huxley/TerminidBase.h"


ATerminidBase::ATerminidBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ATerminidBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATerminidBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ATerminidBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

