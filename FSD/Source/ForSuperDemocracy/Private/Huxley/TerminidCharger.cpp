#include "Huxley/TerminidCharger.h"
#include "Huxley/TerminidFSM.h"
#include "Huxley/TerminidScavenger.h"
#include "Huxley/TerminidWarrior.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"
#include "Engine/DamageEvents.h"
#include "Engine/OverlapResult.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

ATerminidCharger::ATerminidCharger()
{
    // 차저 기본 설정
    ChargeDistance = 800.0f;
    ChargeSpeedMultiplier = 3.0f;
    ChargeDamageMultiplier = 2.5f;
    ChargeStunRadius = 200.0f;
    ChargeCooldown = 5.0f;
    ArmorDamageReduction = 0.3f;
    HurtResistance = 0.7f;
    bCanBreakWalls = true;
    ThreatRadius = 500.0f;
    bCausesFearEffect = true;
    FearEffectRadius = 300.0f;
    
    // 내부 상태 초기화
    bIsCharging = false;
    bChargeOnCooldown = false;
    ChargeStartTime = 0.0f;
    LastChargeTime = 0.0f;
    ChargeStartLocation = FVector::ZeroVector;
    ChargeTargetLocation = FVector::ZeroVector;
    ChargeCurrentSpeed = 0.0f;
    bHasHitTarget = false;
    bChargeCompleted = false;
    LastThreatCheckTime = 0.0f;
    LastFearEffectTime = 0.0f;
    bIsCausingFear = false;
    LastWallCheckTime = 0.0f;
    
    // 차저는 더 큰 충돌 크기
    GetCapsuleComponent()->SetCapsuleHalfHeight(120.0f);
    GetCapsuleComponent()->SetCapsuleRadius(50.0f);
}

void ATerminidCharger::BeginPlay()
{
    Super::BeginPlay();
    
    // 차저 전용 스탯으로 초기화
    FTerminidStats ChargerStats = FTerminidStats::CreateChargerStats();
    InitializeTerminid(ChargerStats, ETerminidType::Charger);
}

// 상태 확인 함수들
bool ATerminidCharger::IsCharging() const
{
    return bIsCharging;
}

bool ATerminidCharger::CanPerformCharge() const
{
    if (bChargeOnCooldown || bIsCharging || !HasValidTarget())
        return false;
    
    float ChargeDistanceToTarget = GetChargeDistanceToTarget();
    return ChargeDistanceToTarget >= CHARGE_MIN_DISTANCE && ChargeDistanceToTarget <= ChargeDistance;
}

bool ATerminidCharger::IsInChargeRange() const
{
    float ChargeDistanceToTarget = GetChargeDistanceToTarget();
    return ChargeDistanceToTarget <= ChargeDistance && ChargeDistanceToTarget >= CHARGE_MIN_DISTANCE;
}

float ATerminidCharger::GetChargeDistanceToTarget() const
{
    if (!HasValidTarget())
        return 0.0f;
        
    return FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
}

void ATerminidCharger::StartChargeAttack()
{
    if (!CanPerformCharge())
        return;
    
    bIsCharging = true;
    bHasHitTarget = false;
    bChargeCompleted = false;
    ChargeStartTime = GetWorld()->GetTimeSeconds();
    ChargeStartLocation = GetActorLocation();
    
    CalculateChargeTarget();
    
    // 이동 속도 대폭 증가
    UCharacterMovementComponent* CharMovement = GetCharacterMovement();
    if (CharMovement)
    {
        ChargeCurrentSpeed = BaseStats.MoveSpeed * ChargeSpeedMultiplier;
        CharMovement->MaxWalkSpeed = ChargeCurrentSpeed;
    }
    
    // Blueprint 이벤트 호출
    OnChargeStart();
    
    // 공포 효과 시작
    if (bCausesFearEffect)
    {
        CauseFearEffect();
    }
}

void ATerminidCharger::StopChargeAttack()
{
    if (!bIsCharging)
        return;
    
    bIsCharging = false;
    bChargeCompleted = true;
    LastChargeTime = GetWorld()->GetTimeSeconds();
    bChargeOnCooldown = true;
    
    // 이동 속도 원상복구
    UCharacterMovementComponent* CharMovement = GetCharacterMovement();
    if (CharMovement)
    {
        CharMovement->MaxWalkSpeed = BaseStats.MoveSpeed;
    }
    
    // 돌진 후 잠시 스턴
    StateMachine->EnterHurtState(POST_CHARGE_STUN_DURATION);
    
    // 쿨다운 타이머 시작
    GetWorld()->GetTimerManager().SetTimer(
        ChargeRecoveryTimerHandle,
        [this]() { bChargeOnCooldown = false; },
        ChargeCooldown,
        false
    );
    
    // Blueprint 이벤트 호출
    OnChargeEnd();
}

void ATerminidCharger::ExecuteChargeImpact()
{
    if (!bIsCharging)
        return;
    
    bHasHitTarget = true;
    
    // 충격 범위 내의 모든 액터에게 데미지
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    
    bool bHit = World->OverlapMultiByChannel(
        OverlapResults,
        GetActorLocation(),
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(ChargeStunRadius),
        QueryParams
    );
    
    if (bHit)
    {
        for (const FOverlapResult& Result : OverlapResults)
        {
            AActor* HitActor = Result.GetActor();
            if (HitActor && HitActor != this)
            {
                // 데미지 적용
                ATerminidBase* HitTerminid = Cast<ATerminidBase>(HitActor);
                if (HitTerminid)
                {
                    float ChargeDamage = BaseStats.AttackDamage * ChargeDamageMultiplier;
                    FPointDamageEvent DamageEvent(ChargeDamage, FHitResult(), GetActorForwardVector(), nullptr);
                    HitTerminid->TakeDamage(ChargeDamage, DamageEvent, nullptr, this);
                }
                
                // Blueprint 이벤트 호출
                OnChargeImpact(HitActor);
            }
        }
    }
    
    // 벽 파괴 체크
    if (bCanBreakWalls)
    {
        BreakNearbyWalls();
    }
    
    StopChargeAttack();
}

void ATerminidCharger::CauseFearEffect()
{
    if (!bCausesFearEffect)
        return;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastFearEffectTime < FEAR_EFFECT_INTERVAL)
        return;
    
    LastFearEffectTime = CurrentTime;
    bIsCausingFear = true;
    
    ApplyFearToNearbyEnemies();
    
    // Blueprint 이벤트 호출
    OnFearEffectTriggered();
    
    // 공포 효과 지속 시간
    GetWorld()->GetTimerManager().SetTimer(
        FearEffectTimerHandle,
        [this]() { bIsCausingFear = false; },
        2.0f,
        false
    );
}

void ATerminidCharger::BreakNearbyWalls()
{
    // 환경 오브젝트 파괴 로직
    CheckWallBreaking();
}

// 행동 오버라이드
void ATerminidCharger::ProcessIdleBehavior(float DeltaTime)
{
    Super::ProcessIdleBehavior(DeltaTime);
    
    UpdateThreatDetection(DeltaTime);
    
    // 차저는 대기 중에도 주변에 위협감 조성
    if (bCausesFearEffect)
    {
        CauseFearEffect();
    }
}

void ATerminidCharger::ProcessChaseBehavior(float DeltaTime)
{
    UpdateThreatDetection(DeltaTime);
    
    // 돌진 공격 가능 거리 확인
    if (CanPerformCharge() && IsInChargeRange())
    {
        StartChargeAttack();
        return;
    }
    
    // 돌진 중인 경우
    if (bIsCharging)
    {
        UpdateChargeAttack(DeltaTime);
        return;
    }
    
    // 일반 추적 (느리지만 꾸준히)
    Super::ProcessChaseBehavior(DeltaTime);
}

void ATerminidCharger::ProcessAttackBehavior(float DeltaTime)
{
    UpdateThreatDetection(DeltaTime);
    
    // 근접 공격 또는 돌진 공격 선택
    if (IsTargetInAttackRange())
    {
        // 일반 공격
        Super::ProcessAttackBehavior(DeltaTime);
    }
    else if (CanPerformCharge() && IsInChargeRange())
    {
        // 돌진 공격
        StartChargeAttack();
    }
    else
    {
        // 추적 상태로 전환
        StateMachine->ChangeState(ETerminidState::Chase);
    }
}

void ATerminidCharger::ProcessHurtBehavior(float DeltaTime)
{
    // 차저는 스턴 저항력이 높음
    if (FMath::RandRange(0.0f, 1.0f) > HurtResistance)
    {
        Super::ProcessHurtBehavior(DeltaTime);
    }
    else
    {
        // 스턴 저항으로 빠르게 회복
        StateMachine->RevertToPreviousState();
    }
}

// 전투 오버라이드
bool ATerminidCharger::CanPerformAttack() const
{
    // 돌진 중에는 일반 공격 불가
    if (bIsCharging)
        return false;
    
    return Super::CanPerformAttack();
}

void ATerminidCharger::PerformAttack()
{
    if (!CanPerformAttack())
        return;
    
    // 차저의 강력한 일반 공격
    Super::PerformAttack();
    
    // 공격 시 작은 범위 충격 효과
    if (HasValidTarget() && IsTargetInAttackRange())
    {
        float SmallImpactRadius = ChargeStunRadius * 0.3f;
        
        UWorld* World = GetWorld();
        if (World)
        {
            TArray<FOverlapResult> OverlapResults;
            FCollisionQueryParams QueryParams;
            QueryParams.AddIgnoredActor(this);
            
            bool bHit = World->OverlapMultiByChannel(
                OverlapResults,
                GetActorLocation(),
                FQuat::Identity,
                ECC_Pawn,
                FCollisionShape::MakeSphere(SmallImpactRadius),
                QueryParams
            );
            
            if (bHit)
            {
                for (const FOverlapResult& Result : OverlapResults)
                {
                    AActor* HitActor = Result.GetActor();
                    if (HitActor && HitActor != this && HitActor != CurrentTarget)
                    {
                        ATerminidBase* HitTerminid = Cast<ATerminidBase>(HitActor);
                        if (HitTerminid)
                        {
                            float SplashDamage = BaseStats.AttackDamage * 0.5f;
                            FPointDamageEvent DamageEvent(SplashDamage, FHitResult(), GetActorForwardVector(), nullptr);
                            HitTerminid->TakeDamage(SplashDamage, DamageEvent, nullptr, this);
                        }
                    }
                }
            }
        }
    }
}

void ATerminidCharger::Die()
{
    // 죽을 때 마지막 폭발적 공격
    if (HasValidTarget())
    {
        ExecuteChargeImpact();
    }
    
    // 주변 Terminid들에게 분노 효과 부여
    InfluenceNearbyTerminids();
    
    Super::Die();
}

float ATerminidCharger::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    // 장갑으로 인한 데미지 감소
    float ReducedDamage = Damage * (1.0f - ArmorDamageReduction);
    
    return Super::TakeDamage(ReducedDamage, DamageEvent, EventInstigator, DamageCauser);
}

// 내부 함수들
void ATerminidCharger::UpdateChargeAttack(float DeltaTime)
{
    if (!bIsCharging)
        return;
    
    ProcessChargeMovement(DeltaTime);
    
    // 목표 지점 도달 또는 충돌 확인
    float ChargeDistanceRemaining = FVector::Dist(GetActorLocation(), ChargeTargetLocation);
    if (ChargeDistanceRemaining <= 100.0f || bHasHitTarget)
    {
        ExecuteChargeImpact();
    }
    
    // 벽 파괴 체크
    if (bCanBreakWalls)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastWallCheckTime >= WALL_CHECK_INTERVAL)
        {
            LastWallCheckTime = CurrentTime;
            CheckWallBreaking();
        }
    }
}

void ATerminidCharger::UpdateThreatDetection(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if (CurrentTime - LastThreatCheckTime >= THREAT_CHECK_INTERVAL)
    {
        LastThreatCheckTime = CurrentTime;
        
        // 주변 적들에게 위협 효과
        if (bCausesFearEffect)
        {
            ApplyFearToNearbyEnemies();
        }
    }
}

void ATerminidCharger::ProcessChargeMovement(float DeltaTime)
{
    UCharacterMovementComponent* CharMovement = GetCharacterMovement();
    if (!CharMovement || !bIsCharging)
        return;
    
    FVector Direction = (ChargeTargetLocation - GetActorLocation()).GetSafeNormal();
    
    // 가속도 적용
    ChargeCurrentSpeed = FMath::FInterpTo(
        ChargeCurrentSpeed,
        BaseStats.MoveSpeed * ChargeSpeedMultiplier,
        DeltaTime,
        CHARGE_ACCELERATION / 1000.0f
    );
    
    CharMovement->MaxWalkSpeed = ChargeCurrentSpeed;
    CharMovement->AddInputVector(Direction);
    
    // 돌진 방향으로 빠른 회전
    if (!Direction.IsZero())
    {
        FRotator TargetRotation = Direction.Rotation();
        TargetRotation.Pitch = 0.0f;
        
        FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 10.0f);
        SetActorRotation(NewRotation);
    }
}

void ATerminidCharger::CalculateChargeTarget()
{
    if (!HasValidTarget())
    {
        ChargeTargetLocation = GetActorLocation() + GetActorForwardVector() * ChargeDistance;
        return;
    }
    
    // 타겟의 현재 위치와 예상 이동 경로 고려
    FVector TargetLocation = CurrentTarget->GetActorLocation();
    FVector TargetVelocity = FVector::ZeroVector;
    
    // 타겟의 속도 정보가 있다면 예측 위치 계산
    APawn* TargetPawn = Cast<APawn>(CurrentTarget);
    if (TargetPawn)
    {
        TargetVelocity = TargetPawn->GetVelocity();
    }
    
    // 돌진 시간 예측
    float ChargeTime = ChargeDistance / (BaseStats.MoveSpeed * ChargeSpeedMultiplier);
    FVector PredictedLocation = TargetLocation + (TargetVelocity * ChargeTime);
    
    ChargeTargetLocation = PredictedLocation;
}

void ATerminidCharger::HandleChargeCollision()
{
    // 충돌 처리 로직
    ExecuteChargeImpact();
}

void ATerminidCharger::ApplyFearToNearbyEnemies()
{
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    // 근처의 다른 타입 Terminid들에게 공포 효과 (도주 유발)
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ATerminidBase::StaticClass(), FoundActors);
    
    FVector MyLocation = GetActorLocation();
    
    for (AActor* Actor : FoundActors)
    {
        ATerminidBase* OtherTerminid = Cast<ATerminidBase>(Actor);
        if (OtherTerminid && OtherTerminid != this && OtherTerminid->IsAlive())
        {
            float Distance = FVector::Dist(MyLocation, OtherTerminid->GetActorLocation());
            if (Distance <= FearEffectRadius)
            {
                // 스캐빈저에게 특히 강한 공포 효과
                ATerminidScavenger* Scavenger = Cast<ATerminidScavenger>(OtherTerminid);
                if (Scavenger && Scavenger->GetHealthPercent() < 0.7f)
                {
                    Scavenger->StateMachine->ChangeState(ETerminidState::Flee);
                }
            }
        }
    }
}

void ATerminidCharger::InfluenceNearbyTerminids()
{
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    // 죽을 때 주변 Terminid들에게 분노 효과 부여
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ATerminidBase::StaticClass(), FoundActors);
    
    FVector MyLocation = GetActorLocation();
    
    for (AActor* Actor : FoundActors)
    {
        ATerminidBase* OtherTerminid = Cast<ATerminidBase>(Actor);
        if (OtherTerminid && OtherTerminid != this && OtherTerminid->IsAlive())
        {
            float Distance = FVector::Dist(MyLocation, OtherTerminid->GetActorLocation());
            if (Distance <= ThreatRadius)
            {
                // 워리어들을 광폭화 상태로 만듦
                ATerminidWarrior* Warrior = Cast<ATerminidWarrior>(OtherTerminid);
                if (Warrior)
                {
                    Warrior->EnterBerserkMode();
                }
                
                // 타겟 공유
                if (!OtherTerminid->HasValidTarget() && HasValidTarget())
                {
                    OtherTerminid->SetCurrentTarget(CurrentTarget);
                    OtherTerminid->StateMachine->ChangeState(ETerminidState::Chase);
                }
            }
        }
    }
}

void ATerminidCharger::CheckWallBreaking()
{
    if (!bCanBreakWalls)
        return;
    
    // 환경 오브젝트 파괴 로직 (벽, 장애물 등)
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    
    bool bHit = World->OverlapMultiByChannel(
        OverlapResults,
        GetActorLocation(),
        FQuat::Identity,
        ECC_WorldStatic,
        FCollisionShape::MakeSphere(100.0f),
        QueryParams
    );
    
    if (bHit)
    {
        for (const FOverlapResult& Result : OverlapResults)
        {
            AActor* HitActor = Result.GetActor();
            if (HitActor && !DestroyedObjects.Contains(HitActor))
            {
                // 파괴 가능한 오브젝트인지 확인 후 파괴
                // 실제 구현은 게임의 환경 시스템에 따라 달라짐
                DestroyedObjects.Add(HitActor);
                
                // Blueprint 이벤트 호출
                OnWallBreak(HitActor);
            }
        }
    }
}

void ATerminidCharger::DestroyEnvironmentObjects()
{
    CheckWallBreaking();
}