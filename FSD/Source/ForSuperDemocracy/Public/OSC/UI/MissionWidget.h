// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MissionWidget.generated.h"

/**
 * 
 */
class UTextBlock;
UCLASS()
class FORSUPERDEMOCRACY_API UMissionWidget : public UUserWidget
{
	GENERATED_BODY()

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MissionTitle;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MissionSource;

public:
	UFUNCTION(BlueprintCallable)
	void SetMissionTitle(FText text);

	UFUNCTION(BlueprintCallable)
	void SetMissionSource(FText text);
};
