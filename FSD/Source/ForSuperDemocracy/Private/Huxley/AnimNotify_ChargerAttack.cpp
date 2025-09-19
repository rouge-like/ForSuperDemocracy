#include "Huxley/AnimNotify_ChargerAttack.h"
#include "Huxley/TerminidCharger.h"
#include "OSC/HealthComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/DamageEvents.h"

UAnimNotify_ChargerAttack::UAnimNotify_ChargerAttack()
{
    // Charger 기본값 설정 - 강력한 범위 공격
    DamageAmount = 60.0f;
    AttackRange = 500.0f;
    AttackAngle = 120.0f;
    KnockbackForce = 800.0f;
    KnockbackUpForce = 200.0f;
    bShowDebug = true;
    DebugDrawTime = 2.0f;

#if WITH_EDITORONLY_DATA
    // 에디터에서 보여질 색상 설정 (빨간색 - Charger)
    NotifyColor = FColor::Red;
#endif
}

void UAnimNotify_ChargerAttack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (!MeshComp || !MeshComp->GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("AnimNotify_ChargerAttack: Invalid MeshComp or World"));
        return;
    }

    // 공격자(Charger) 가져오기
    AActor* Attacker = MeshComp->GetOwner();
    if (!Attacker)
    {
        UE_LOG(LogTemp, Warning, TEXT("AnimNotify_ChargerAttack: No valid attacker found"));
        return;
    }

    // Charger인지 확인
    ATerminidCharger* ChargerAttacker = Cast<ATerminidCharger>(Attacker);
    if (!ChargerAttacker)
    {
        UE_LOG(LogTemp, Warning, TEXT("AnimNotify_ChargerAttack: Attacker is not a TerminidCharger"));
        return;
    }

    UWorld* World = MeshComp->GetWorld();
    FVector AttackerLocation = Attacker->GetActorLocation();
    FVector AttackerForward = Attacker->GetActorForwardVector();

    // 디버그 표시
    if (bShowDebug)
    {
        DrawDebugAttackRange(World, AttackerLocation, AttackerForward);
    }

    // 공격 범위 내 모든 타겟 찾기 (Charger는 범위 공격)
    TArray<AActor*> Targets = FindTargetsInRange(AttackerLocation, AttackerForward, World);

    if (Targets.Num() > 0)
    {
        // 실제 데미지는 Charger의 AttackDamage 스탯 사용
        float ActualDamage = ChargerAttacker->BaseStats.AttackDamage;

        for (AActor* Target : Targets)
        {
            if (Target)
            {
                // 데미지 적용
                ApplyDamageToTarget(Target, Attacker, ActualDamage);

                // 넉백 효과 적용
                ApplyKnockbackToTarget(Target, AttackerLocation);

                UE_LOG(LogTemp, Log, TEXT("AnimNotify_ChargerAttack: Applied %f damage to %s with knockback"),
                       ActualDamage, *Target->GetName());
            }
        }

        UE_LOG(LogTemp, Log, TEXT("AnimNotify_ChargerAttack: Hit %d targets"), Targets.Num());
    }
}

TArray<AActor*> UAnimNotify_ChargerAttack::FindTargetsInRange(const FVector& AttackerLocation, const FVector& AttackerForward, UWorld* World)
{
    TArray<AActor*> TargetsInRange;

    if (!World)
        return TargetsInRange;

    // 모든 플레이어를 대상으로 검색 (Charger는 범위 공격이므로 모든 타겟 수집)
    for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
    {
        APlayerController* PC = Iterator->Get();
        if (PC && PC->GetPawn())
        {
            AActor* PotentialTarget = PC->GetPawn();
            FVector TargetLocation = PotentialTarget->GetActorLocation();

            if (IsTargetInAttackRange(AttackerLocation, AttackerForward, TargetLocation))
            {
                TargetsInRange.Add(PotentialTarget);
            }
        }
    }

    return TargetsInRange;
}

bool UAnimNotify_ChargerAttack::IsTargetInAttackRange(const FVector& AttackerLocation, const FVector& AttackerForward, const FVector& TargetLocation)
{
    // 거리 체크
    float Distance = FVector::Dist(AttackerLocation, TargetLocation);
    if (Distance > AttackRange)
    {
        return false;
    }

    // 각도 체크
    FVector ToTarget = (TargetLocation - AttackerLocation).GetSafeNormal();
    float DotProduct = FVector::DotProduct(AttackerForward, ToTarget);
    float AngleCos = FMath::Cos(FMath::DegreesToRadians(AttackAngle / 2.0f));

    return DotProduct >= AngleCos;
}

void UAnimNotify_ChargerAttack::ApplyDamageToTarget(AActor* Target, AActor* Attacker, float Damage)
{
    if (!Target || !Attacker)
        return;

    // 1. 먼저 HealthComponent를 통한 데미지 적용 시도
    UHealthComponent* HealthComp = Target->FindComponentByClass<UHealthComponent>();
    if (HealthComp)
    {
        AController* AttackerController = nullptr;
        if (APawn* AttackerPawn = Cast<APawn>(Attacker))
        {
            AttackerController = AttackerPawn->GetController();
        }

        HealthComp->ApplyDamage(Damage, Attacker, AttackerController, DamageTypeClass);
        return;
    }

    // 2. HealthComponent가 없으면 언리얼 엔진 기본 데미지 시스템 사용
    FPointDamageEvent DamageEvent;
    DamageEvent.Damage = Damage;
    DamageEvent.HitInfo.Location = Target->GetActorLocation();
    DamageEvent.ShotDirection = (Target->GetActorLocation() - Attacker->GetActorLocation()).GetSafeNormal();

    AController* AttackerController = nullptr;
    if (APawn* AttackerPawn = Cast<APawn>(Attacker))
    {
        AttackerController = AttackerPawn->GetController();
    }

    Target->TakeDamage(Damage, DamageEvent, AttackerController, Attacker);
}

void UAnimNotify_ChargerAttack::ApplyKnockbackToTarget(AActor* Target, const FVector& AttackerLocation)
{
    if (!Target)
        return;

    // Character의 MovementComponent를 통한 넉백
    ACharacter* TargetCharacter = Cast<ACharacter>(Target);
    if (TargetCharacter && TargetCharacter->GetCharacterMovement())
    {
        FVector KnockbackDirection = (Target->GetActorLocation() - AttackerLocation).GetSafeNormal();
        FVector KnockbackVelocity = KnockbackDirection * KnockbackForce + FVector::UpVector * KnockbackUpForce;

        TargetCharacter->GetCharacterMovement()->AddImpulse(KnockbackVelocity, true);
        return;
    }

    // Pawn의 RootComponent를 통한 넉백 (Character가 아닌 경우)
    APawn* TargetPawn = Cast<APawn>(Target);
    if (TargetPawn && TargetPawn->GetRootComponent())
    {
        UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(TargetPawn->GetRootComponent());
        if (RootPrimitive && RootPrimitive->IsSimulatingPhysics())
        {
            FVector KnockbackDirection = (Target->GetActorLocation() - AttackerLocation).GetSafeNormal();
            FVector KnockbackVelocity = KnockbackDirection * KnockbackForce + FVector::UpVector * KnockbackUpForce;

            RootPrimitive->AddImpulse(KnockbackVelocity, NAME_None, true);
        }
    }
}

void UAnimNotify_ChargerAttack::DrawDebugAttackRange(UWorld* World, const FVector& AttackerLocation, const FVector& AttackerForward)
{
    if (!World)
        return;

    // 공격 범위를 부채꼴로 표시 (빨간색 - Charger)
    FVector RightVector = FVector::CrossProduct(AttackerForward, FVector::UpVector);
    float HalfAngle = AttackAngle / 2.0f;

    // 중앙선 (더 굵게)
    DrawDebugLine(World, AttackerLocation,
                  AttackerLocation + AttackerForward * AttackRange,
                  FColor::Red, false, DebugDrawTime, 0, 3.0f);

    // 좌우 경계선
    FVector LeftBound = AttackerForward.RotateAngleAxis(-HalfAngle, FVector::UpVector);
    FVector RightBound = AttackerForward.RotateAngleAxis(HalfAngle, FVector::UpVector);

    DrawDebugLine(World, AttackerLocation,
                  AttackerLocation + LeftBound * AttackRange,
                  FColor::Orange, false, DebugDrawTime, 0, 2.0f);

    DrawDebugLine(World, AttackerLocation,
                  AttackerLocation + RightBound * AttackRange,
                  FColor::Orange, false, DebugDrawTime, 0, 2.0f);

    // 호 그리기 (부채꼴 모양) - 더 많은 선으로 넓은 범위 표현
    for (float Angle = -HalfAngle; Angle <= HalfAngle; Angle += 3.0f)
    {
        FVector Direction1 = AttackerForward.RotateAngleAxis(Angle, FVector::UpVector);
        FVector Direction2 = AttackerForward.RotateAngleAxis(Angle + 3.0f, FVector::UpVector);

        DrawDebugLine(World,
                      AttackerLocation + Direction1 * AttackRange,
                      AttackerLocation + Direction2 * AttackRange,
                      FColor::Yellow, false, DebugDrawTime, 0, 1.0f);
    }

    // 공격자 위치 표시 (더 크게)
    DrawDebugSphere(World, AttackerLocation, 25.0f, 12, FColor::Red, false, DebugDrawTime);

    // 넉백 방향 표시 (화살표 대신 선으로)
    for (int i = 0; i < 8; i++)
    {
        float RadialAngle = (360.0f / 8.0f) * i;
        FVector RadialDirection = AttackerForward.RotateAngleAxis(RadialAngle, FVector::UpVector);
        FVector EndPoint = AttackerLocation + RadialDirection * (AttackRange * 0.7f);

        // 화살표 대신 굵은 선으로 표시
        DrawDebugLine(World, AttackerLocation, EndPoint,
                      FColor::Purple, false, DebugDrawTime, 0, 2.0f);

        // 화살표 끝부분 표시 (작은 구체)
        DrawDebugSphere(World, EndPoint, 5.0f, 6, FColor::Purple, false, DebugDrawTime);
    }
}