// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/Weapon/WeaponBase.h"
#include "OSC/Weapon/WeaponData.h"
#include "OSC/Weapon/WeaponComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"

// 기본값 설정
AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
}

// BeginPlay: 스폰/시작 시 초기화
void AWeaponBase::BeginPlay()
{
    Super::BeginPlay();
    // WeaponData가 있다면 시작 탄약을 데이터 기준으로 초기화
    if (Data)
    {
        CurrentAmmo = Data->MaxSize;
    }
}

// Called every frame
void AWeaponBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // 조준 정렬은 캐릭터/AnimBP에서 처리(무기 틱에서는 트레이스하지 않음)
    FiringTime += DeltaTime;

    // 시간 경과에 따른 블룸 회복
    if (Data && CurrentBloom > 0.f)
    {
        const float Recover = Data->SpreadRecoveryPerSec * DeltaTime;
        CurrentBloom = FMath::Max(0.f, CurrentBloom - Recover);
    }

    
    if (bIsFiring && Data && FiringTime >= Data->FireTime)
    {
        FireOnce();
    }

    float RecoverRatio = Data->RecoverDegPerSec * DeltaTime * (bIsFiring ? Data->RecoverWhileFiring : 1.f);


    if (!PC) return;
    // Pitch 복구
    float StepP = FMath::Min(RecoilPitchToRecover, RecoverRatio);
    PC->AddPitchInput(StepP);
    RecoilPitchToRecover -= StepP;
    
    // Yaw 복구
    float SignY = FMath::Sign(RecoilYawToRecover);
    float StepY = FMath::Min(FMath::Abs(RecoilYawToRecover), RecoverRatio);
    PC->AddYawInput(-SignY * StepY);
    RecoilYawToRecover -= SignY * StepY;
    
}

void AWeaponBase::RegisterWeaponComponent(UWeaponComponent* wc)
{
    WC = wc;

    ACharacter* Character =  Cast<ACharacter>(GetOwner());
    if (Character)
    {
        PC = Cast<APlayerController>(Character->GetController());
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
    if (bIsReloading || !Data)
        return;
    
    if (CurrentAmmo >= Data->MaxSize)
        return;

    if (WC->GetReserveAmmo(Data->AmmoType) <= 0)
        return;

    bIsReloading = true;

    GetWorldTimerManager().SetTimer(TimerHandle_Reload, this, &AWeaponBase::EndReload, Data->ReloadTime, false);
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
    
    // 탄약 소모
    CurrentAmmo = FMath::Max(0, CurrentAmmo - 1);

    // 단일 라인트레이스로 피격 판정 및 데미지 적용
    FVector TraceStart = GetMuzzleLocation();
    FRotator ViewRot = GetFireRotation();

    if (const APawn* P = Cast<APawn>(GetOwner()))
    {
        if (const AController* C = P->GetController())
        {
            C->GetPlayerViewPoint(TraceStart, ViewRot);
        }
    }
    

    // 조준 방향 주변 원뿔(콘) 내에서 스프레드를 적용해 발사 방향 산출
    FVector AimDir = ViewRot.Vector();
    float SpreadDeg = Data ? GetCurrentSpreadDegrees() : 0.f;
    if (SpreadDeg > KINDA_SMALL_NUMBER)
    {
        const float SpreadRad = FMath::DegreesToRadians(SpreadDeg);
        AimDir = FMath::VRandCone(AimDir, SpreadRad);
    }
    const FVector TraceEnd = TraceStart + AimDir * MaxRange;

    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(WeaponFireTrace), false, this);
    Params.AddIgnoredActor(this);
    if (GetOwner()) Params.AddIgnoredActor(GetOwner());

    const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, Params);
    const FVector ImpactPoint = bHit ? Hit.ImpactPoint : TraceEnd;

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
    // DrawDebugLine(GetWorld(), TraceStart, ImpactPoint, bHit ? FColor::Red : FColor::Green, false, 1.0f, 0, 1.5f);
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
        UGameplayStatics::ApplyPointDamage(HitActor, Data ? Data->Damage : 0.f, ShotDir, Hit, InstigatorController, this, UDamageType::StaticClass());
        
    }

    ShowBullet();

    // 반동을 카메라/컨트롤러 입력에 적용
    ApplyRecoilKick();

    // 블룸 증가(연사 페널티 누적)
    if (Data)
    {
        CurrentBloom = FMath::Min(Data->SpreadMax, CurrentBloom + Data->SpreadIncreasePerShot);
    }
}

float AWeaponBase::GetCurrentSpreadDegrees() const
{
    if (!Data) return 0.f;
    return Data->BaseSpread + CurrentBloom;
}

void AWeaponBase::ApplyRecoilKick()
{
    if (!Data || !PC) return;

    const float PitchKick = FMath::FRandRange(Data->RecoilPitchMin, Data->RecoilPitchMax);
    const float YawAbs = FMath::FRandRange(Data->RecoilYawMin, Data->RecoilYawMax);
    const float YawKick = (FMath::RandBool() ? 1.f : -1.f) * YawAbs;

    PC->AddPitchInput(-PitchKick); // 위로 차오르는 Pitch 반동
    PC->AddYawInput(YawKick);      // 좌/우 미세 Yaw 반동

    RecoilPitchToRecover += PitchKick;
    RecoilYawToRecover += YawKick;
}
