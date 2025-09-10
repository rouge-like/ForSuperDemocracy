// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/MainHUD.h"

#include "Blueprint/UserWidget.h"

void AMainHUD::BeginPlay()
{
	Super::BeginPlay();

	if (MainUIClass)
	{
		MainUI = CreateWidget<UUserWidget>(GetWorld(), MainUIClass);
		if (MainUI)
		{
			MainUI->AddToViewport(0);
		}
	}
}

void AMainHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (MainUI)
	{
		MainUI->RemoveFromParent();
		MainUI = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}