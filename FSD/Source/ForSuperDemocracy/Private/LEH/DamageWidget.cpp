// Fill out your copyright notice in the Description page of Project Settings.


#include "LEH/DamageWidget.h"

void UDamageWidget::NativeConstruct()
{
	Super::NativeConstruct();

	
}

void UDamageWidget::PlayFadeIn()
{
	PlayAnimation(FadeIn);
}

void UDamageWidget::PlayFadeOut()
{
	PlayAnimation(FadeOut);
}
