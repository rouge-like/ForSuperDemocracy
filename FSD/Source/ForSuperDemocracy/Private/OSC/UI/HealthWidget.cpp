// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/UI/HealthWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UHealthWidget::SetHealthBar(float Ratio)
{
	HealthBar->SetPercent(Ratio);
}

void UHealthWidget::SetHealCount(int32 CurrentCnt, int32 MaxCnt)
{
	FText count = FText::FromString(FString::Printf(TEXT("%d / %d"), CurrentCnt, MaxCnt));
	HealCount->SetText(count);
}
