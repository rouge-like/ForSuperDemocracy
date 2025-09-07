// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AmmoType.h"
#include "WeaponData.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FAimViewParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ADS")
	float TargetFOV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ADS")
	float TargetArmLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ADS")
	FVector SocketOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ADS")
	float InterpSpeed;
};
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

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FAimViewParams viewParams;

    // Recoil parameters
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Recoil")
    float RecoilPitchMin = 0.2f; // degrees per shot

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Recoil")
    float RecoilPitchMax = 0.6f; // degrees per shot

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Recoil")
    float RecoilYawMin = 0.0f; // degrees per shot

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Recoil")
    float RecoilYawMax = 0.4f; // degrees per shot

    // Multiplier applied while aiming down sights (typically < 1.0)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Recoil")
    float RecoilADSScalar = 0.6f;

    // Spread/Bloom parameters (degrees)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Spread")
    float BaseSpreadHip = 1.2f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Spread")
    float BaseSpreadADS = 0.3f;

    // How much spread increases per shot (degrees)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Spread")
    float SpreadIncreasePerShot = 0.2f;

    // Max additional bloom (degrees)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Spread")
    float SpreadMax = 3.0f;

    // Bloom recovery per second (degrees)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Spread")
    float SpreadRecoveryPerSec = 3.0f;
};
