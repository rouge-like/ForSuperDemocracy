// Fill out your copyright notice in the Description page of Project Settings.

#include "OSC/Weapon/WeaponComponent.h"
#include "OSC/Weapon/WeaponData.h"
#include "OSC/Weapon/AmmoType.h"
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
        if (weaponList.Num() > 0)
        {
            currentIdx = 0;
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
	
	currentIdx = idx;
}

void UWeaponComponent::StartFire()
{
	weaponList[currentIdx]->StartFire();
}

void UWeaponComponent::StopFire()
{
	weaponList[currentIdx]->StopFire();
}

void UWeaponComponent::Reload()
{
	weaponList[currentIdx]->StartReload();
}

int32 UWeaponComponent::PullAmmo(EAmmoType type, int32 need)
{
    int32* poolPtr = AmmoPools.Find(type);
    int32& pool = poolPtr ? * poolPtr : AmmoPools.Add(type, 0);
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
    weaponList.AddUnique(Weapon);
}

int32 UWeaponComponent::GetReserveAmmo(EAmmoType type)
{
	int32* poolPtr = AmmoPools.Find(type);
	int32& pool = poolPtr ? * poolPtr : AmmoPools.Add(type, 0);
	return pool;
}
