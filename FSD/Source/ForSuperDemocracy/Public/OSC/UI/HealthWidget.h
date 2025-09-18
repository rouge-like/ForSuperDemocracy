// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthWidget.generated.h"

class UProgressBar;
class UTextBlock;
/**
 * 
 */
UCLASS()
class FORSUPERDEMOCRACY_API UHealthWidget : public UUserWidget
{
	GENERATED_BODY()
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> HealCount;

public:
	UFUNCTION(BlueprintCallable)
	void SetHealthBar(float Ratio);
	UFUNCTION(BlueprintCallable)
	void SetHealCount(int32 CurrentCnt, int32 MaxCnt = 4);
};
