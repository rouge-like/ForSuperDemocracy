// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainUI.generated.h"

class UWeaponWidget;
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

public:
	UWeaponWidget* GetWeaponWidget(){ return WeaponWidget; }
};
