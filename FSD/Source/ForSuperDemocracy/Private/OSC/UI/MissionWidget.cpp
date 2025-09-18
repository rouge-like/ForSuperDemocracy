// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/UI/MissionWidget.h"
#include "Components/TextBlock.h"

void UMissionWidget::SetMissionTitle(FText text)
{
	MissionTitle->SetText(text);
}

void UMissionWidget::SetMissionSource(FText text)
{
	MissionSource->SetText(text);
}
