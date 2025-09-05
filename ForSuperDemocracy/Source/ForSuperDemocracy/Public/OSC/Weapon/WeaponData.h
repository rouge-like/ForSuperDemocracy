// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AmmoType.h"
#include "WeaponData.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class FORSUPERDEMOCRACY_API UWeaponData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EAmmoType AmmoType;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxSize;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float damage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float fireTime;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float reloadTime;
};
