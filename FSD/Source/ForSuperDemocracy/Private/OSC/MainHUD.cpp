// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/MainHUD.h"
#include "OSC/UI/MainUI.h"
#include "Blueprint/UserWidget.h"
#include "OSC/MissionData.h"
#include "OSC/MainMode.h"
#include "OSC/MissionComponent.h"
#include "OSC/UI/CompassWidget.h"
#include "OSC/UI/MissionWidget.h"

void AMainHUD::BeginPlay()
{
	Super::BeginPlay();

	if (MainUIClass)
	{
		MainUI = CreateWidget<UMainUI>(GetWorld(), MainUIClass);
		if (MainUI)
		{
			MainUI->AddToViewport(0);
		}
	}
	
	if (CinematicUIClass)
	{
		CinematicUI = CreateWidget<UUserWidget>(GetWorld(), CinematicUIClass);
	}
	if (auto* GM = GetWorld()->GetAuthGameMode<AMainMode>())
	{
		auto* MC = GM->GetMissionComponent();
		if (MainUI && MainUI->GetCompassWidget())
		{
			MainUI->GetCompassWidget()->InitializeCompass(MC);
		}

		MC->OnObjectiveChanged.AddDynamic(this, &AMainHUD::OnMissionObjectiveChanged);
		MC->OnObjectiveUpdated.AddDynamic(this, &AMainHUD::OnMissionObjectiveUpdate);
		MC->OnTimerTick.AddDynamic(this, &AMainHUD::OnMissionTimerTick);
		MC->OnMissionCompleted.AddDynamic(this, &AMainHUD::OnMissionComplete);
		MC->StartMission();
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

void AMainHUD::OnMissionObjectiveChanged(const FMissionObjective& Objective)
{
	FString title = "- " + Objective.Label;
	MainUI->GetMissionWidget()->SetMissionTitle(FText::FromString(title));
}

void AMainHUD::OnMissionObjectiveUpdate(int32 Curr, int32 Target)
{
	FString cnt = "[" + FString::FromInt(Curr) + "/" + FString::FromInt(Target) + "]";
	if (Target == 0)
		cnt = "";
	MainUI->GetMissionWidget()->SetMissionSource(FText::FromString(cnt));
}

void AMainHUD::OnMissionComplete()
{
	if (MainUI)
	{
		MainUI->RemoveFromParent();
	}
	if (CinematicUI)
	{
		CinematicUI->AddToViewport(0);
	}
}

void AMainHUD::OnMissionTimerTick(int32 RemainSec)
{
	int32 min = RemainSec / 60;
	int32 sec = RemainSec % 60;
	FString minStr;
	FString secStr;
	FString time;
	if (sec > 9)
	{
		secStr = FString::FromInt(sec);
	}
	else
	{
		secStr = "0" + FString::FromInt(sec);
	}
	if (min > 0)
	{
		minStr = "0" + FString::FromInt(min)+ ":";
	}
	else
	{
		minStr = "00:";
	}
	time = minStr  + secStr;
	MainUI->GetMissionWidget()->SetMissionSource(FText::FromString(time));
}
