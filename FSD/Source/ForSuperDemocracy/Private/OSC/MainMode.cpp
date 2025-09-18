// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/MainMode.h"

#include "OSC/MissionComponent.h"

AMainMode::AMainMode()
{
	MissionComponent = CreateDefaultSubobject<UMissionComponent>("MissionComponent");
}

void AMainMode::BeginPlay()
{
	Super::BeginPlay();
	
}
