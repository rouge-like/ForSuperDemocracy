// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/Weapon/WeaponBase.h"
#include "OSC/Weapon/WeaponData.h"
#include "OSC/Weapon/WeaponComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// Sets default values
AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
    Super::BeginPlay();
    // Initialize ammo from data if available
    if (data)
    {
        currentAmmo = data->MaxSize;
    }
}

// Called every frame
void AWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    firingTime += DeltaTime;

    if (bIsFiring && firingTime >= data->fireTime)
    {
        FireOnce();
    }
}

void AWeaponBase::StartFire()
{
    if (bIsReloading)
    {
        return;
    }
    bIsFiring = true;
    FireOnce();
}

void AWeaponBase::StopFire()
{
    bIsFiring = false;
}

void AWeaponBase::StartReload()
{
    if (bIsReloading || !data)
        return;
    
    if (currentAmmo >= data->MaxSize)
        return;
    
    bIsReloading = true;

    GetWorldTimerManager().SetTimer(TimerHandle_Reload, this, &AWeaponBase::EndReload, data->reloadTime, false);
    if(GEngine) GEngine->AddOnScreenDebugMessage(1, 1.5f, FColor::Green, FString::Printf(TEXT("Reloading...")));
}

bool AWeaponBase::CanFire() const
{
    return !bIsReloading && data && currentAmmo > 0;
}

FVector AWeaponBase::GetMuzzleLocation() const
{
    if (Mesh && Mesh->DoesSocketExist(MuzzleSocketName))
    {
        return Mesh->GetSocketLocation(MuzzleSocketName);
    }
    return GetActorLocation();
}

FRotator AWeaponBase::GetFireRotation() const
{
    if (bUseOwnerView)
    {
        if (const APawn* PawnOwner = Cast<APawn>(GetOwner()))
        {
            FVector L;
            FRotator R;
            if (const AController* C = PawnOwner->GetController())
            {
                C->GetPlayerViewPoint(L, R);
                return R;
            }
        }
    }

    if (Mesh && Mesh->DoesSocketExist(MuzzleSocketName))
    {
        return Mesh->GetSocketRotation(MuzzleSocketName);
    }
    return GetActorRotation();
}

void AWeaponBase::EndReload()
{
    // Pull ammo from owner's weapon component pool
    int32 Need = data->MaxSize - currentAmmo;
    int32 Pulled = 0;
    if (AActor* OwnerActor = GetOwner())
    {
        if (UWeaponComponent* WC = OwnerActor->FindComponentByClass<UWeaponComponent>())
        {
            Pulled = WC->PullAmmo(data->AmmoType, Need);
        }
    }
    currentAmmo += Pulled;

    bIsReloading = false;

    ShowBullet();
}

void AWeaponBase::ShowBullet() const
{
    if (AActor* OwnerActor = GetOwner())
    {
        if (UWeaponComponent* WC = OwnerActor->FindComponentByClass<UWeaponComponent>())
        {
            if(GEngine) GEngine->AddOnScreenDebugMessage(1, 1.5f, FColor::Green, FString::Printf(TEXT("Bullet %d / %d"), GetCurrentAmmo(), WC->GetReserveAmmo(data->AmmoType)));
        }
    }
}

void AWeaponBase::FireOnce()
{
    if (!CanFire())
        return;

    firingTime = 0;
    
    // Consume ammo
    currentAmmo = FMath::Max(0, currentAmmo - 1);

    const FVector Start = bUseOwnerView ? ( [this]()
    {
        FVector L;
        FRotator R;
        if(const APawn* P=Cast<APawn>(GetOwner()))
        {
            if(const AController* C=P->GetController())
            {
                C->GetPlayerViewPoint(L,R);
                return L;
            }
        }
        return GetMuzzleLocation();
    } ()) : GetMuzzleLocation();
    const FRotator Rot = GetFireRotation();
    const FVector End = Start + Rot.Vector() * MaxRange;

    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(WeaponTrace), false, this);
    Params.AddIgnoredActor(this);
    if (GetOwner()) Params.AddIgnoredActor(GetOwner());

    bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_Visibility, Params);

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
    DrawDebugLine(GetWorld(), Start, bHit ? Hit.ImpactPoint : End, bHit ? FColor::Red : FColor::Green, false, 1.0f, 0, 1.0f);
    DrawDebugLine(GetWorld(), GetMuzzleLocation(), bHit ? Hit.ImpactPoint : End, bHit ? FColor::Red : FColor::Green, false, 1.0f, 0, 1.0f);
#endif

    if (bHit && Hit.GetActor())
    {
        const FVector ShotDir = -Rot.Vector();
        UGameplayStatics::ApplyPointDamage(Hit.GetActor(), data ? data->damage : 10.f, ShotDir, Hit, GetInstigatorController(), this, UDamageType::StaticClass());
    }
    ShowBullet();
}


