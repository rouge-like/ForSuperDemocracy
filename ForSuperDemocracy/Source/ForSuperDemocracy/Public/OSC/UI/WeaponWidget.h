// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WeaponWidget.generated.h"

/**
 * 
 */
class UTextBlock;
UCLASS()
class FORSUPERDEMOCRACY_API UWeaponWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CurrentAmmo;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MaxAmmo;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CurrentGrenade;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MaxGrenade;

public:
	void InitWeapons(int32 currentAmmo, int32 maxAmmo, int32 currentGrenade, int32 maxGrenade);
	void SetCurrentAmmo(int32 ammo);
	void SetMaxAmmo(int32 ammo);
	void SetGrenade(int32 grenade);
};
