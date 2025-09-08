// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/Weapon/ProjectileWeapon.h"

void AProjectileWeapon::FireOnce()
{
	if (!CanFire())
		return;
	Super::FireOnce();
	
}
