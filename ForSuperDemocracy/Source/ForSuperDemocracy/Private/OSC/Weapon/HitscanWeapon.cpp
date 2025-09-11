// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/Weapon/HitscanWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequenceBase.h"
#include "NiagaraFunctionLibrary.h"

void AHitscanWeapon::FireOnce()
{
	if (!CanFire())
		return;
	
    Super::FireOnce();

    // 무기 메시에서 애니메이션 시퀀스를 직접 재생(ABP/몽타주 없이)
    if (Mesh && FireAnim)
    {
        // 이전 복구 타이머가 있으면 정리(연사 중 덮어쓰기)
        GetWorldTimerManager().ClearTimer(TimerHandle_RestoreAnim);

        // 기존 모드/클래스를 캐시한 뒤 SingleNode로 전환하여 재생
        CachedAnimMode = Mesh->GetAnimationMode();
        if (CachedAnimMode == EAnimationMode::AnimationBlueprint)
        {
            CachedAnimClass = Mesh->GetAnimClass();
        }
        Mesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
        Mesh->PlayAnimation(FireAnim, /*bLoop*/ false);

        const float Duration = FMath::Max(0.01f, FireAnim->GetPlayLength());
        GetWorldTimerManager().SetTimer(TimerHandle_RestoreAnim, this, &AHitscanWeapon::RestoreWeaponAnimMode, Duration, false);
    }
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

	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECollisionChannel::ECC_GameTraceChannel1, Params);
	const FVector ImpactPoint = bHit ? Hit.ImpactPoint : TraceEnd;
	
	if (ShotVFX)
	{
		// Use muzzle socket world rotation so VFX forward aligns with barrel
		FRotator MuzzleRot = GetActorRotation();
		FVector MuzzleLoc = GetMuzzleLocation();
		if (Mesh && Mesh->DoesSocketExist(MuzzleSocketName))
		{
			const FTransform MuzzleTM = Mesh->GetSocketTransform(MuzzleSocketName, ERelativeTransformSpace::RTS_World);
			MuzzleLoc = MuzzleTM.GetLocation();
			MuzzleRot = MuzzleTM.Rotator();
		}
		// Apply optional asset-specific correction
		MuzzleRot += ShotVFXRotationOffset;

		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ShotVFX,
			MuzzleLoc,
			MuzzleRot,
			FVector(0.01f)
		);
	}
	
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	// DrawDebugLine(GetWorld(), TraceStart, ImpactPoint, bHit ? FColor::Red : FColor::Green, false, 1.0f, 0, 1.5f);
	DrawDebugLine(GetWorld(), GetMuzzleLocation(), ImpactPoint, bHit ? FColor::Red : FColor::Green, false, 1.0f, 0, 1.5f);
	DrawDebugSphere(GetWorld(),ImpactPoint, 10, 0, FColor::Yellow, false, 1.0f, 0);
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
}

void AHitscanWeapon::RestoreWeaponAnimMode()
{
    if (!Mesh)
        return;

    if (CachedAnimMode == EAnimationMode::AnimationBlueprint)
    {
        Mesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
        if (CachedAnimClass)
        {
            Mesh->SetAnimInstanceClass(CachedAnimClass);
        }
    }
    else
    {
        Mesh->SetAnimationMode(CachedAnimMode);
    }
}
