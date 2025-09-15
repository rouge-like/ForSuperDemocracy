// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OSC/Weapon/WeaponBase.h"
#include "ProjectileWeapon.generated.h"

/**
 * 
 */
class AProjectileBase;
UCLASS()
class FORSUPERDEMOCRACY_API AProjectileWeapon : public AWeaponBase
{
	GENERATED_BODY()

protected:
	virtual void FireOnce() override;

	UPROPERTY(EditDefaultsOnly, Category="Projectile");
	TSubclassOf<AProjectileBase> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category="Projectile");
	float InitialSpeed = 1600.f;
};
