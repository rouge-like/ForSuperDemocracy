// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainUI.generated.h"

class UMissionWidget;
class UWeaponWidget;
class UHealthWidget;
class UCompassWidget;
/**
 * 
 */
UCLASS()
class FORSUPERDEMOCRACY_API UMainUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWeaponWidget> WeaponWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHealthWidget> HealthWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UMissionWidget> MissionWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCompassWidget> CompassWidget;
	
public:
	UFUNCTION(BlueprintCallable)
	UWeaponWidget* GetWeaponWidget(){ return WeaponWidget; }
	UFUNCTION(BlueprintCallable)
	UHealthWidget* GetHealthWidget(){ return HealthWidget; }
	UFUNCTION(BlueprintCallable)
	UMissionWidget* GetMissionWidget(){ return MissionWidget; }
	UFUNCTION(BlueprintCallable)
	UCompassWidget* GetCompassWidget(){ return CompassWidget; }
};
