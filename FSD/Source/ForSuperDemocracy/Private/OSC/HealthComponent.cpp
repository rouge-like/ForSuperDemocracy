// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/HealthComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/EngineTypes.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "TimerManager.h"
#include "GameFramework/SpringArmComponent.h"

// 컴포넌트 기본값 설정(틱 활성화 등)
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// BeginPlay: 체력 초기화 및 데미지 콜백 연결
void UHealthComponent::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth;

    if (AActor* Owner = GetOwner())
    {
        Owner->SetCanBeDamaged(true);
        Owner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::HandleAnyDamage);
        Owner->OnTakePointDamage.AddDynamic(this, &UHealthComponent::HandlePointDamage);
        Owner->OnTakeRadialDamage.AddDynamic(this, &UHealthComponent::HandleRadialDamage);
    }
}


// 매 프레임 호출(필요 시 HUD 동기화 등 확장 가능)
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (bIsRagdolling && bSyncCapsuleToRagdoll)
    {
        UpdateCapsuleFollowRagdoll(DeltaTime);
    }
}

void UHealthComponent::Heal(float Amount)
{
    if (Amount <= 0.f || CurrentHealth <= 0.f)
        return;

    const float Old = CurrentHealth;
    CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.f, MaxHealth);
    OnHealthChanged.Broadcast(CurrentHealth, CurrentHealth - Old);
}

// 외부에서 데미지 유도 시 사용(엔진 ApplyDamage를 통해 내부 이벤트 흐름으로 연결)
void UHealthComponent::ApplyDamage(float Amount, AActor* DamageCauser, AController* InstigatedBy, TSubclassOf<UDamageType> DamageType)
{
    if (Amount <= 0.f)
        return;

    if (AActor* Owner = GetOwner())
    {
        ApplyDamageInternal(Amount, DamageCauser, InstigatedBy, nullptr);
        //UGameplayStatics::ApplyDamage(Owner, Amount, InstigatedBy, DamageCauser, DamageType);
    }
}

// 현재 체력만큼 내부 데미지 처리 → 즉시 사망 유도
void UHealthComponent::Kill()
{
    if (CurrentHealth <= 0.f)
        return;

    ApplyDamageInternal(CurrentHealth, /*DamageCauser*/nullptr, /*InstigatedBy*/nullptr, /*DamageType*/nullptr);
}

// 엔진 AnyDamage 콜백 → 내부 공통 처리로 위임
void UHealthComponent::HandleAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
    // ApplyDamageInternal(Damage, DamageCauser, InstigatedBy, DamageType);
}

// 포인트 데미지(탄환 등) 콜백 → 내부 공통 처리로 위임
void UHealthComponent::HandlePointDamage(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser)
{
    if (HitParticle)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticle, HitLocation);
    }
   
    ApplyDamageInternal(Damage, DamageCauser, InstigatedBy, DamageType);         
}

// 반경 데미지(폭발 등) 콜백 → 내부 공통 처리로 위임
void UHealthComponent::HandleRadialDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, FVector Origin, const FHitResult& HitInfo, AController* InstigatedBy, AActor* DamageCauser)
{
    ApplyDamageInternal(Damage, DamageCauser, InstigatedBy, DamageType);

    // Step 1: 간단한 래그돌 + 임펄스 적용 (데미지 비례)
    if (Damage <= 0.f)
    {
        return;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }
    USkeletalMeshComponent* Mesh = nullptr;
    if (ACharacter* Char = Cast<ACharacter>(Owner))
    {
        Mesh = Char->GetMesh();
    }
    if (!Mesh)
    {
        Mesh = Cast<USkeletalMeshComponent>(Owner->GetComponentByClass(USkeletalMeshComponent::StaticClass()));
    }
    if (!Mesh)
    {
        return;
    }

    OnRagdoll();
    // 데미지 비례 임펄스 적용 (Origin 반대 방향으로 가속 변화)
    FVector Dir = Mesh->GetComponentLocation() - Origin - FVector(0,0,500.f);
    if (!Dir.IsNearlyZero())
    {
        Dir = Dir.GetSafeNormal();
    }
    const float Strength = Damage * ImpulsePerDamage;
    Mesh->AddImpulse(Dir * Strength, NAME_None, /*bVelChange*/ true);
}

void UHealthComponent::UpdateCapsuleFollowRagdoll(float /*DeltaTime*/)
{
    AActor* Owner = GetOwner();
    ACharacter* Char = Owner ? Cast<ACharacter>(Owner) : nullptr;
    if (!Char)
    {
        return;
    }

    USkeletalMeshComponent* Mesh = Char->GetMesh();
    UCapsuleComponent* Capsule = Char->GetCapsuleComponent();
    if (!Mesh || !Capsule)
    {
        return;
    }

    FVector Pelvis = Mesh->GetBoneLocation(RagdollPelvisBoneName, EBoneSpaces::WorldSpace);
    const float HalfHeight = Capsule->GetUnscaledCapsuleHalfHeight();
    if (Pelvis.IsNearlyZero())
    {
        Pelvis = Mesh->GetSocketLocation(RagdollPelvisBoneName);
        Pelvis.Z += HalfHeight + 0.5f;
    }
    FVector Target = Pelvis;
    Target.Z -= HalfHeight;
    Target.Z += RagdollCapsuleFollowZOffset;

    Owner->SetActorLocation(Target, false, nullptr, ETeleportType::TeleportPhysics);

    if (bSyncCapsuleYawToPelvis)
    {
        const FRotator PelvisRot = Mesh->GetBoneQuaternion(RagdollPelvisBoneName, EBoneSpaces::WorldSpace).Rotator();
        const FRotator NewRot(0.f, PelvisRot.Yaw, 0.f);
        Owner->SetActorRotation(NewRot, ETeleportType::TeleportPhysics);
    }
}

void UHealthComponent::OnRagdoll()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }
    USkeletalMeshComponent* Mesh = nullptr;
    if (ACharacter* Char = Cast<ACharacter>(Owner))
    {
        Mesh = Char->GetMesh();
    }
    if (!Mesh)
    {
        Mesh = Cast<USkeletalMeshComponent>(Owner->GetComponentByClass(USkeletalMeshComponent::StaticClass()));
    }
    if (!Mesh)
    {
        return;
    }

    // 래그돌 활성화(최소 설정)
    if (!Mesh->IsSimulatingPhysics())
    {
        PrevMeshCollisionProfileName = Mesh->GetCollisionProfileName();

        // Detach mesh from capsule to avoid parent influence during ragdoll
        if (ACharacter* CharOwner = Cast<ACharacter>(Owner))
        {
            SavedMeshRelativeTransform = Mesh->GetRelativeTransform();
            Mesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
            bDetachedMeshDuringRagdoll = true;
        }
        Mesh->SetCollisionProfileName(TEXT("Ragdoll"));
        Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Mesh->SetSimulatePhysics(true);
        Mesh->SetAllBodiesSimulatePhysics(true);
        Mesh->WakeAllRigidBodies();
        bIsRagdolling = true;

        // 캡슐 콜리전 비활성 + 이동 정지/불가 처리 (Character에 한함)
        if (ACharacter* Char = Cast<ACharacter>(Owner))
        {
            if (UCapsuleComponent* Capsule = Char->GetCapsuleComponent())
            {
                Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                Capsule->SetEnableGravity(false);
            }
            if (UCharacterMovementComponent* MoveComp = Char->GetCharacterMovement())
            {
                MoveComp->StopMovementImmediately();
                PrevGravityScale = MoveComp->GravityScale;
                MoveComp->GravityScale = 0.f;
                MoveComp->SetMovementMode(MOVE_None);
            }
            if (USpringArmComponent* SpringArm = Cast<USpringArmComponent>(Char->GetComponentByClass(USpringArmComponent::StaticClass())))
            {
                SpringArm->TargetOffset *= 2;
            }
        }

        // Broadcast ragdoll start for external systems (e.g., camera follow)
        OnRagdollStart.Broadcast(Owner);

        // 일정 시간 후 간단 복귀
        if (RagdollRecoverTime > 0.f)
        {
            FTimerHandle Handle;
            GetWorld()->GetTimerManager().SetTimer(Handle, this, &UHealthComponent::RecoverFromRagdoll, RagdollRecoverTime, false);
        }
    }
}

void UHealthComponent::RecoverFromRagdoll()
{
    if (CurrentHealth <= 0.f)
        return;
    
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    USkeletalMeshComponent* Mesh = nullptr;
    if (ACharacter* Char = Cast<ACharacter>(Owner))
    {
        Mesh = Char->GetMesh();
        if (USpringArmComponent* SpringArm = Cast<USpringArmComponent>(Char->GetComponentByClass(USpringArmComponent::StaticClass())))
        {
            SpringArm->TargetOffset /= 2;
        }
    }
    if (!Mesh)
    {
        Mesh = Cast<USkeletalMeshComponent>(Owner->GetComponentByClass(USkeletalMeshComponent::StaticClass()));
    }
    if (!Mesh)
    {
        bIsRagdolling = false;
        return;
    }

    Mesh->SetSimulatePhysics(false);
    Mesh->SetAllBodiesSimulatePhysics(false);
    if (!PrevMeshCollisionProfileName.IsNone())
    {
        Mesh->SetCollisionProfileName(PrevMeshCollisionProfileName);
    }
    else
    {
        Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    bIsRagdolling = false;

    // 캡슐 위치 스냅 및 콜리전/이동 복구 (Character에 한함)
    if (ACharacter* Char = Cast<ACharacter>(Owner))
    {
        UCapsuleComponent* Capsule = Char->GetCapsuleComponent();
        UCharacterMovementComponent* MoveComp = Char->GetCharacterMovement();
        if (Capsule)
        {
            // 간단 위치 보정: pelvis 아래로 반높이만큼 배치
            FVector Pelvis = Mesh->GetBoneLocation(RagdollPelvisBoneName, EBoneSpaces::WorldSpace);
            const float HalfHeight = Capsule->GetUnscaledCapsuleHalfHeight();
            if (Pelvis.IsNearlyZero())
            {
                Pelvis = Mesh->GetSocketLocation(RagdollPelvisBoneName);
                Pelvis.Z += HalfHeight + 0.5f;
            }
            FVector Target = Pelvis;
            Target.Z -= HalfHeight;
            Owner->SetActorLocation(Target, false, nullptr, ETeleportType::TeleportPhysics);

            // 캡슐 콜리전/이동 복구
            Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            Capsule->SetEnableGravity(true);
        }
        if (MoveComp)
        {
            MoveComp->GravityScale = PrevGravityScale;
            MoveComp->SetMovementMode(MOVE_Walking);
        }

        // 메시 재부착 및 상대 트랜스폼 복원
        if (bDetachedMeshDuringRagdoll && Capsule)
        {
            Mesh->AttachToComponent(Capsule, FAttachmentTransformRules::KeepRelativeTransform);
            Mesh->SetRelativeTransform(SavedMeshRelativeTransform);
            bDetachedMeshDuringRagdoll = false;
        }
    }
    
    
    // Broadcast ragdoll end for external systems (e.g., camera follow)
    OnRagdollEnd.Broadcast(Owner);
}

// 공통 데미지 처리: 체력 감소, 이벤트 브로드캐스트, 사망 처리
void UHealthComponent::ApplyDamageInternal(float Damage, AActor* DamageCauser, AController* InstigatedBy, const UDamageType* DamageType)
{
    if (Damage <= 0.f)
        return;

    if (CurrentHealth <= 0.f)
        return;

    const float Old = CurrentHealth;
    CurrentHealth = FMath::Clamp(Old - Damage, 0.f, MaxHealth);

    OnDamaged.Broadcast(Damage, DamageCauser, InstigatedBy, DamageType ? DamageType->GetClass() : nullptr);
    OnHealthChanged.Broadcast(MaxHealth, CurrentHealth);

    if (CurrentHealth <= 0.f && bCanDie)
    {
        OnDeath.Broadcast(GetOwner());
        OnRagdoll();
    }

}

void UHealthComponent::ResetHealth()
{
    CurrentHealth = MaxHealth;
    OnHealthChanged.Broadcast(MaxHealth, CurrentHealth);
    RecoverFromRagdoll();
}
