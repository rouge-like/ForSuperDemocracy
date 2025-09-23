#include "Huxley/AnimNotify_ScavengerAttack.h"
#include "Huxley/TerminidBase.h"
#include "OSC/HealthComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/DamageEvents.h"

UAnimNotify_ScavengerAttack::UAnimNotify_ScavengerAttack()
{
    // 기본값 설정
    DamageAmount = 25.0f;
    AttackRange = 120.0f;
    AttackAngle = 90.0f;
    bShowDebug = false;
    DebugDrawTime = 2.0f;

#if WITH_EDITORONLY_DATA
    // 에디터에서 보여질 색상 설정
    NotifyColor = FColor::Red;
#endif
}

void UAnimNotify_ScavengerAttack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (!MeshComp || !MeshComp->GetWorld())
    {
        return;
    }

    // 공격자(Terminid) 가져오기
    AActor* Attacker = MeshComp->GetOwner();
    if (!Attacker)
    {
        return;
    }

    // Terminid인지 확인
    ATerminidBase* TerminidAttacker = Cast<ATerminidBase>(Attacker);
    if (!TerminidAttacker)
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
        // 실제 데미지는 Terminid의 AttackDamage 스탯 사용
        float ActualDamage = TerminidAttacker->BaseStats.AttackDamage;
        
        ApplyDamageToTarget(Target, Attacker, ActualDamage);
    }
}

AActor* UAnimNotify_ScavengerAttack::FindTargetInRange(const FVector& AttackerLocation, const FVector& AttackerForward, UWorld* World)
{
    if (!World)
        return nullptr;

    AActor* ClosestTarget = nullptr;
    float ClosestDistance = AttackRange + 1.0f; // 범위 밖으로 초기화

    // 모든 플레이어를 대상으로 검색
    for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
    {
        APlayerController* PC = Iterator->Get();
        if (PC && PC->GetPawn())
        {
            AActor* PotentialTarget = PC->GetPawn();
            FVector TargetLocation = PotentialTarget->GetActorLocation();

            // 공격 범위 및 각도 내에 있는지 확인
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

bool UAnimNotify_ScavengerAttack::IsTargetInAttackRange(const FVector& AttackerLocation, const FVector& AttackerForward, const FVector& TargetLocation)
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

void UAnimNotify_ScavengerAttack::ApplyDamageToTarget(AActor* Target, AActor* Attacker, float Damage)
{
    if (!Target || !Attacker)
        return;

    // 1. 먼저 HealthComponent를 통한 데미지 적용 시도
    UHealthComponent* HealthComp = Target->FindComponentByClass<UHealthComponent>();
    if (HealthComp)
    {
        // HealthComponent가 있으면 직접 데미지 적용
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

void UAnimNotify_ScavengerAttack::DrawDebugAttackRange(UWorld* World, const FVector& AttackerLocation, const FVector& AttackerForward)
{
    if (!World)
        return;

    // 공격 범위를 부채꼴로 표시
    FVector RightVector = FVector::CrossProduct(AttackerForward, FVector::UpVector);
    float HalfAngle = AttackAngle / 2.0f;

    // 중앙선
    DrawDebugLine(World, AttackerLocation, 
                  AttackerLocation + AttackerForward * AttackRange, 
                  FColor::Red, false, DebugDrawTime, 0, 2.0f);

    // 좌우 경계선
    FVector LeftBound = AttackerForward.RotateAngleAxis(-HalfAngle, FVector::UpVector);
    FVector RightBound = AttackerForward.RotateAngleAxis(HalfAngle, FVector::UpVector);

    DrawDebugLine(World, AttackerLocation, 
                  AttackerLocation + LeftBound * AttackRange, 
                  FColor::Orange, false, DebugDrawTime, 0, 1.0f);

    DrawDebugLine(World, AttackerLocation, 
                  AttackerLocation + RightBound * AttackRange, 
                  FColor::Orange, false, DebugDrawTime, 0, 1.0f);

    // 호 그리기 (부채꼴 모양)
    for (float Angle = -HalfAngle; Angle <= HalfAngle; Angle += 5.0f)
    {
        FVector Direction1 = AttackerForward.RotateAngleAxis(Angle, FVector::UpVector);
        FVector Direction2 = AttackerForward.RotateAngleAxis(Angle + 5.0f, FVector::UpVector);
        
        DrawDebugLine(World, 
                      AttackerLocation + Direction1 * AttackRange,
                      AttackerLocation + Direction2 * AttackRange,
                      FColor::Yellow, false, DebugDrawTime, 0, 1.0f);
    }

    // 공격자 위치 표시
    DrawDebugSphere(World, AttackerLocation, 20.0f, 8, FColor::Red, false, DebugDrawTime);
}