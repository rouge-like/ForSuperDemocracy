// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DamageWidget.generated.h"

/**
 * 
 */
UCLASS()
class FORSUPERDEMOCRACY_API UDamageWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta=(BindWidgetAnim), Transient)
	UWidgetAnimation* FadeIn;

	UPROPERTY(meta=(BindWidgetAnim), Transient)
	UWidgetAnimation* FadeOut;

public:
	UFUNCTION(BlueprintCallable)
	void PlayFadeIn();

	UFUNCTION(BlueprintCallable)
	void PlayFadeOut();
};
