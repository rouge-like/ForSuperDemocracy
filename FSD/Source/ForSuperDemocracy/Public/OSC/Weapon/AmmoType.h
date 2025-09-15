#pragma once

#include "AmmoType.generated.h"

UENUM(BlueprintType)
enum class EAmmoType : uint8
{
	None UMETA(DisplayName = "None"),
	PistolAmmo UMETA(DisplayName = "PistolAmmo"),
	RifleAmmo UMETA(DisplayName = "RifleAmmo"),
	Grenade UMETA(DisplayName = "Grenade")
};
