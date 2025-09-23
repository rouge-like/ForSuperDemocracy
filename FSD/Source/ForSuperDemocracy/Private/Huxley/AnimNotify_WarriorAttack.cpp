#include "Huxley/AnimNotify_WarriorAttack.h"
#include "Huxley/TerminidWarrior.h"
#include "OSC/HealthComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/DamageEvents.h"

UAnimNotify_WarriorAttack::UAnimNotify_WarriorAttack()
{
    // Warrior 기본값 설정 - 빠르고 정확한 공격
    DamageAmount = 35.0f;
    AttackRange = 360.0f;
    AttackAngle = 60.0f;
    bShowDebug = true;
    DebugDrawTime = 2.0f;

#if WITH_EDITORONLY_DATA
    // 에디터에서 보여질 색상 설정 (파란색 - Warrior)
    NotifyColor = FColor::Blue;
#endif
}

void UAnimNotify_WarriorAttack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (!MeshComp || !MeshComp->GetWorld())
    {
        return;
    }

    // 공격자(Warrior) 가져오기
    AActor* Attacker = MeshComp->GetOwner();
    if (!Attacker)
    {
        return;
    }

    // Warrior인지 확인
    ATerminidWarrior* WarriorAttacker = Cast<ATerminidWarrior>(Attacker);
    if (!WarriorAttacker)
    {
        return;
    }

    // 회피 중에는 공격하지 않음
    if (WarriorAttacker->IsDodging())
    {
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

    // 공격 범위 내 타겟 찾기
    AActor* Target = FindTargetInRange(AttackerLocation, AttackerForward, World);

    if (Target)
    {
        // 실제 데미지는 Warrior의 AttackDamage 스탯 사용
        float ActualDamage = WarriorAttacker->BaseStats.AttackDamage;

        ApplyDamageToTarget(Target, Attacker, ActualDamage);
    }
}

AActor* UAnimNotify_WarriorAttack::FindTargetInRange(const FVector& AttackerLocation, const FVector& AttackerForward, UWorld* World)
{
    if (!World)
        return nullptr;

    AActor* ClosestTarget = nullptr;
    float ClosestDistance = AttackRange + 1.0f;

    // 모든 플레이어를 대상으로 검색
    for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
    {
        APlayerController* PC = Iterator->Get();
        if (PC && PC->GetPawn())
        {
            AActor* PotentialTarget = PC->GetPawn();
            FVector TargetLocation = PotentialTarget->GetActorLocation();

            if (IsTargetInAttackRange(AttackerLocation, AttackerForward, TargetLocation))
            {
                float Distance = FVector::Dist(AttackerLocation, TargetLocation);
                if (Distance < ClosestDistance)
                {
                    ClosestDistance = Distance;
                    ClosestTarget = PotentialTarget;
                }
            }
        }
    }

    return ClosestTarget;
}

bool UAnimNotify_WarriorAttack::IsTargetInAttackRange(const FVector& AttackerLocation, const FVector& AttackerForward, const FVector& TargetLocation)
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

void UAnimNotify_WarriorAttack::ApplyDamageToTarget(AActor* Target, AActor* Attacker, float Damage)
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

void UAnimNotify_WarriorAttack::DrawDebugAttackRange(UWorld* World, const FVector& AttackerLocation, const FVector& AttackerForward)
{
    if (!World)
        return;

    // 공격 범위를 부채꼴로 표시 (파란색 - Warrior)
    FVector RightVector = FVector::CrossProduct(AttackerForward, FVector::UpVector);
    float HalfAngle = AttackAngle / 2.0f;

    // 중앙선
    DrawDebugLine(World, AttackerLocation,
                  AttackerLocation + AttackerForward * AttackRange,
                  FColor::Blue, false, DebugDrawTime, 0, 2.0f);

    // 좌우 경계선
    FVector LeftBound = AttackerForward.RotateAngleAxis(-HalfAngle, FVector::UpVector);
    FVector RightBound = AttackerForward.RotateAngleAxis(HalfAngle, FVector::UpVector);

    DrawDebugLine(World, AttackerLocation,
                  AttackerLocation + LeftBound * AttackRange,
                  FColor::Cyan, false, DebugDrawTime, 0, 1.0f);

    DrawDebugLine(World, AttackerLocation,
                  AttackerLocation + RightBound * AttackRange,
                  FColor::Cyan, false, DebugDrawTime, 0, 1.0f);

    // 호 그리기 (부채꼴 모양)
    for (float Angle = -HalfAngle; Angle <= HalfAngle; Angle += 5.0f)
    {
        FVector Direction1 = AttackerForward.RotateAngleAxis(Angle, FVector::UpVector);
        FVector Direction2 = AttackerForward.RotateAngleAxis(Angle + 5.0f, FVector::UpVector);

        DrawDebugLine(World,
                      AttackerLocation + Direction1 * AttackRange,
                      AttackerLocation + Direction2 * AttackRange,
                      FColor::Green, false, DebugDrawTime, 0, 1.0f);
    }

    // 공격자 위치 표시
    DrawDebugSphere(World, AttackerLocation, 15.0f, 8, FColor::Blue, false, DebugDrawTime);
}