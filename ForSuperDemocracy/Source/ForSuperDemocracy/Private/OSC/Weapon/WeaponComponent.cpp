// Fill out your copyright notice in the Description page of Project Settings.

#include "OSC/Weapon/WeaponComponent.h"
#include "OSC/Weapon/WeaponBase.h"
#include "Components/ChildActorComponent.h"


// Sets default values for this component's properties
UWeaponComponent::UWeaponComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UWeaponComponent::BeginPlay()
{
    Super::BeginPlay();

    // ...
    if (AActor* OwnerActor = GetOwner())
    {
        TArray<UChildActorComponent*> ChildActors;
        OwnerActor->GetComponents<UChildActorComponent>(ChildActors);
        for (UChildActorComponent* CAC : ChildActors)
        {
            if (!CAC) continue;
            if (AActor* Child = CAC->GetChildActor())
            {
                if (AWeaponBase* Weapon = Cast<AWeaponBase>(Child))
                {
                    RegisterWeapon(Weapon);
                }
            }
        }
        if (WeaponList.Num() > 0)
        {
            CurrentIdx = 0;
        }
    }
}


// Called every frame
void UWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UWeaponComponent::Equip(int32 idx)
{
	StopFire();
	
	CurrentIdx = idx;
}

void UWeaponComponent::StartFire()
{
	if (!bIsAiming)
		return;
	
	WeaponList[CurrentIdx]->StartFire();
}

void UWeaponComponent::StopFire()
{
	WeaponList[CurrentIdx]->StopFire();
}

void UWeaponComponent::Reload()
{
	WeaponList[CurrentIdx]->StartReload();
}

int32 UWeaponComponent::PullAmmo(EAmmoType type, int32 need)
{
    int32& pool = AmmoPools.FindOrAdd(type);
    const int32 give = FMath::Clamp(need, 0, pool);

    pool -= give;

    return give;
}


void UWeaponComponent::RegisterWeapon(AWeaponBase* Weapon)
{
    if (!IsValid(Weapon)) return;
    if (Weapon->GetOwner() != GetOwner())
    {
        Weapon->SetOwner(GetOwner());
    }
	Weapon->RegisterWeaponComponent(this);
    WeaponList.AddUnique(Weapon);
	
}

int32 UWeaponComponent::GetReserveAmmo(EAmmoType type)
{
    int32& pool = AmmoPools.FindOrAdd(type);
    return pool;
}

void UWeaponComponent::StartAiming()
{
	bIsAiming = true;

	WeaponList[CurrentIdx]->SetAiming(bIsAiming);
}

void UWeaponComponent::StopAiming()
{
	bIsAiming = false;

	StopFire();
	WeaponList[CurrentIdx]->SetAiming(bIsAiming);
}
