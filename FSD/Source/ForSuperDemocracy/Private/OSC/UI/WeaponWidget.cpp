// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/UI/WeaponWidget.h"

#include "Components/TextBlock.h"

void UWeaponWidget::InitWeapons(int32 currentAmmo, int32 maxAmmo,
                                int32 currentGrenade, int32 maxGrenade)
{
	SetCurrentAmmo(currentAmmo);
	SetMaxAmmo(maxAmmo);
	SetGrenade(currentGrenade);
	MaxGrenade->SetText(FText::Format(FText::FromString(TEXT("/ {0}")), maxGrenade));
}

void UWeaponWidget::SetCurrentAmmo(int32 ammo)
{
	CurrentAmmo->SetText(FText::AsNumber(ammo));
}

void UWeaponWidget::SetMaxAmmo(int32 ammo)
{
	MaxAmmo->SetText(FText::Format(FText::FromString(TEXT("/ {0}")), ammo));
}

void UWeaponWidget::SetGrenade(int32 grenade)
{
	CurrentGrenade->SetText(FText::AsNumber(grenade));
}
