// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/Stratagem/Stratagem.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AStratagem::AStratagem()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// 충돌 이벤트를 받을 수 있도록 설정 (SphereComp는 부모에서 생성)
	if (SphereComp)
	{
		SphereComp->SetNotifyRigidBodyCollision(true);
		SphereComp->SetGenerateOverlapEvents(true);
	}
}

// Called when the game starts or when spawned
void AStratagem::BeginPlay()
{
    Super::BeginPlay();

	// ProjectileMovement 정지(충돌) 델리게이트 바인딩
	if (ProjectileMovement)
	{
		ProjectileMovement->OnProjectileStop.AddDynamic(this, &AStratagem::OnProjectileStopped);
	}
}

// Called every frame
void AStratagem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}

void AStratagem::OnProjectileStopped(const FHitResult& ImpactResult)
{
    if (bHasLandedAndFixingStarted)
    {
        return;
    }

    StartFixingSequence(ImpactResult);
}

void AStratagem::StartFixingSequence(const FHitResult& ImpactResult)
{
    bHasLandedAndFixingStarted = true;

    // 즉시 이동 정지 및 충돌 정리
    if (ProjectileMovement)
    {
        ProjectileMovement->StopMovementImmediately();
        ProjectileMovement->SetUpdatedComponent(nullptr);
    }

    // 지면에 안착하도록 위치/회전 고정 (법선에 맞춰 정렬 시 간단 처리)
    if (ImpactResult.bBlockingHit)
    {
        // 바닥 법선에 어느 정도 수직 정렬 (선택적)
        const FVector Up = ImpactResult.Normal;
        const FRotator AlignRot = Up.Rotation();
        // Z축만 유지하고 싶다면 커스터마이즈 가능
        SetActorRotation(FRotator(0, 0, 0.f));
    }

    // Fixing 애니메이션 1회 재생
    float fixingLen = 0.f;
    if (MeshComp && FixingAnim)
    {
        MeshComp->PlayAnimation(FixingAnim, /*bLooped*/ false);
        fixingLen = FixingAnim->GetPlayLength();
    }

    // 애니메이션 길이 후 고정 상태 전환
    const float Delay = fixingLen > 0.f ? fixingLen : DelayTime;
    FTimerHandle TimerHandle;
    GetWorldTimerManager().SetTimer(TimerHandle, this, &AStratagem::OnFixingFinished, Delay, false);
}

void AStratagem::AirRaid()
{
    AudioComp->Stop();
    AudioComp->DestroyComponent();
    // 폭발 FX 스폰 (Cascade Particle System)
    if (ExplosionVFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            ExplosionVFX,
            GetActorLocation(),
            GetActorRotation()
        );
    }
    if (ExplosionSFX)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSFX, GetActorLocation(), 2, 1, 0, Attenuation);
    }
    
    // 반경 데미지 적용: 반경 내 모든 Actor에 엔진 데미지 전파 → HealthComponent가 이를 수신해 처리
    TArray<AActor*> IgnoreActors;
    IgnoreActors.Add(this);
    AController* InstigatorController = GetInstigatorController();
    UGameplayStatics::ApplyRadialDamage(
        GetWorld(),
        Damage,
        GetActorLocation(),
        Radius,
        /*DamageType*/ nullptr,
        IgnoreActors,
        /*DamageCauser*/ this,
        InstigatorController,
        /*bDoFullDamage*/ true
    );
    
    Destroy();
}

void AStratagem::OnFixingFinished()
{
    // Fixed 애니메이션 무한 반복
    if (MeshComp && FixedAnim)
    {
        MeshComp->PlayAnimation(FixedAnim, /*bLooped*/ true);
    }
    FTimerHandle TimerHandle;
    GetWorldTimerManager().SetTimer(TimerHandle, this, &AStratagem::AirRaid, DelayTime, false);
    
    // Light VFX 스폰 (메시 루트에 부착)
    if (LightVFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAttached(
            LightVFX,
            MeshComp ? MeshComp.Get() : GetRootComponent(),
            NAME_None,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            /*bAutoDestroy*/ true);
    }
    if (LightSFX)
    {
        AudioComp = UGameplayStatics::SpawnSoundAtLocation(GetWorld(), LightSFX, GetActorLocation(), GetActorRotation(), 2, 1, 0, Attenuation);
        AudioComp->Play(0);
    }
}

