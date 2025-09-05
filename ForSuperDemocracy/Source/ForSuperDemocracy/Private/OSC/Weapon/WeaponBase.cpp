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
    if (Data)
    {
        CurrentAmmo = Data->MaxSize;
    }
}

void AWeaponBase::UpdateAimAlignment()
{
    const FVector Start = bUseOwnerView ?  [this]()
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
    } () : GetMuzzleLocation();
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
        Target = Hit.GetActor();
    }
    else
    {
        Target = nullptr;
    }
}

// Called every frame
void AWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    // Aim alignment handled by character/AnimBP; no per-tick weapon trace
    FiringTime += DeltaTime;

    if (bIsFiring && Data && FiringTime >= Data->fireTime)
    {
        FireOnce();
    }
}

void AWeaponBase::RegisterWeaponComponent(UWeaponComponent* wc)
{
    WC = wc;
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
    if (bIsReloading || !Data)
        return;
    
    if (CurrentAmmo >= Data->MaxSize)
        return;

    if (WC->GetReserveAmmo(Data->AmmoType) <= 0)
        return;

    bIsReloading = true;

    GetWorldTimerManager().SetTimer(TimerHandle_Reload, this, &AWeaponBase::EndReload, Data->reloadTime, false);
    if(GEngine) GEngine->AddOnScreenDebugMessage(1, 1.5f, FColor::Green, FString::Printf(TEXT("Reloading...")));
}

bool AWeaponBase::CanFire() const
{
    return !bIsReloading && Data && CurrentAmmo > 0;
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
    int32 Need = Data->MaxSize - CurrentAmmo;
    int32 Pulled = 0;
    
    if (WC)
        Pulled = WC->PullAmmo(Data->AmmoType, Need);
        
    
    CurrentAmmo += Pulled;

    bIsReloading = false;

    ShowBullet();
}

void AWeaponBase::ShowBullet() const
{
    if(GEngine) GEngine->AddOnScreenDebugMessage(1, 1.5f, FColor::Green, FString::Printf(TEXT("Bullet %d / %d"), GetCurrentAmmo(), WC->GetReserveAmmo(Data->AmmoType)));
}

void AWeaponBase::FireOnce()
{
    if (!CanFire())
        return;

    FiringTime = 0;
    
    // Consume ammo
    CurrentAmmo = FMath::Max(0, CurrentAmmo - 1);

    // Perform a single line trace to determine hit and apply damage
    FVector TraceStart = GetMuzzleLocation();
    FRotator ViewRot = GetFireRotation();
    if (bUseOwnerView)
    {
        if (const APawn* P = Cast<APawn>(GetOwner()))
        {
            if (const AController* C = P->GetController())
            {
                C->GetPlayerViewPoint(TraceStart, ViewRot);
            }
        }
    }

    const FVector TraceEnd = TraceStart + ViewRot.Vector() * MaxRange;

    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(WeaponFireTrace), false, this);
    Params.AddIgnoredActor(this);
    if (GetOwner()) Params.AddIgnoredActor(GetOwner());

    const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, Params);
    const FVector ImpactPoint = bHit ? Hit.ImpactPoint : TraceEnd;

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
    DrawDebugLine(GetWorld(), TraceStart, ImpactPoint, bHit ? FColor::Red : FColor::Green, false, 1.0f, 0, 1.5f);
    DrawDebugLine(GetWorld(), GetMuzzleLocation(), ImpactPoint, bHit ? FColor::Red : FColor::Green, false, 1.0f, 0, 1.5f);
#endif

    if (bHit && Hit.GetActor())
    {
        AActor* HitActor = Hit.GetActor();
        const FVector ShotDir = (ImpactPoint - GetMuzzleLocation()).GetSafeNormal();
        AController* InstigatorController = nullptr;
        if (APawn* P = Cast<APawn>(GetOwner()))
        {
            InstigatorController = P->GetController();
        }
        UGameplayStatics::ApplyPointDamage(HitActor, Data ? Data->damage : 0.f, ShotDir, Hit, InstigatorController, this, UDamageType::StaticClass());
        
    }

    ShowBullet();
}

