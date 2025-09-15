#include "Huxley/TerminidWarrior.h"
#include "Huxley/TerminidFSM.h"
#include "Huxley/TerminidScavenger.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

ATerminidWarrior::ATerminidWarrior()
{
    // 워리어 기본 설정
    BerserkHealthThreshold = 0.5f;
    BerserkAttackMultiplier = 1.2f;
    BerserkSpeedMultiplier = 1.1f;
    SwarmCoordinationRadius = 300.0f;
    OptimalSwarmSize = 4;
    bCanLeadSwarm = true;
    bUseTacticalMovement = true;
    FlankingDistance = 200.0f;
    LeadershipRange = 400.0f;
    
    // 내부 상태 초기화
    bIsBerserk = false;
    bIsSwarmLeader = false;
    BerserkActivationTime = 0.0f;
    LastSwarmCheckTime = 0.0f;
    SwarmMemberCount = 0;
    SwarmFormationCenter = FVector::ZeroVector;
    SwarmFormationRadius = 150.0f;
    bIsExecutingFlankManeuver = false;
    FlankingTarget = FVector::ZeroVector;
    LastTacticalUpdateTime = 0.0f;
    LastLeadershipUpdateTime = 0.0f;
    LastCoordinationTime = 0.0f;
}

void ATerminidWarrior::BeginPlay()
{
    Super::BeginPlay();
    
    // 워리어 전용 스탯으로 초기화
    FTerminidStats WarriorStats = FTerminidStats::CreateWarriorStats();
    InitializeTerminid(WarriorStats, ETerminidType::Warrior);
}

// 상태 확인 함수들
bool ATerminidWarrior::IsBerserk() const
{
    return bIsBerserk;
}

bool ATerminidWarrior::IsSwarmLeader() const
{
    return bIsSwarmLeader;
}

int32 ATerminidWarrior::GetSwarmMemberCount() const
{
    return SwarmMemberCount;
}

bool ATerminidWarrior::HasOptimalSwarmSize() const
{
    return SwarmMemberCount >= OptimalSwarmSize;
}

void ATerminidWarrior::EnterBerserkMode()
{
    if (bIsBerserk)
        return;
        
    bIsBerserk = true;
    BerserkActivationTime = GetWorld()->GetTimeSeconds();
    
    // 이동 속도 증가
    UCharacterMovementComponent* CharMovement = GetCharacterMovement();
    if (CharMovement)
    {
        CharMovement->MaxWalkSpeed = BaseStats.MoveSpeed * BerserkSpeedMultiplier;
    }
}

void ATerminidWarrior::ExitBerserkMode()
{
    if (!bIsBerserk)
        return;
        
    bIsBerserk = false;
    
    // 이동 속도 원상복구
    UCharacterMovementComponent* CharMovement = GetCharacterMovement();
    if (CharMovement)
    {
        CharMovement->MaxWalkSpeed = BaseStats.MoveSpeed;
    }
}

void ATerminidWarrior::BecomeSwarmLeader()
{
    if (bCanLeadSwarm && !bIsSwarmLeader)
    {
        bIsSwarmLeader = true;
        CheckForSwarmMembers();
    }
}

void ATerminidWarrior::CoordinateSwarmAttack()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastCoordinationTime < COORDINATION_COOLDOWN)
        return;
        
    LastCoordinationTime = CurrentTime;
    
    if (!bIsSwarmLeader || !HasValidTarget())
        return;
    
    // 군집 멤버들에게 공격 명령
    for (ATerminidBase* Member : SwarmMembers)
    {
        if (IsValid(Member) && Member->IsAlive())
        {
            if (!Member->HasValidTarget())
            {
                Member->SetCurrentTarget(CurrentTarget);
            }
            
            // 공격 상태가 아니면 공격 상태로 변경
            if (Member->StateMachine && !Member->StateMachine->IsInCombatState())
            {
                Member->StateMachine->ChangeState(ETerminidState::Swarm);
            }
        }
    }
}

FVector ATerminidWarrior::CalculateFlankingPosition(AActor* Target) const
{
    if (!Target)
        return GetActorLocation();
    
    FVector TargetLocation = Target->GetActorLocation();
    FVector MyLocation = GetActorLocation();
    FVector ToTarget = (TargetLocation - MyLocation).GetSafeNormal();
    
    // 측면으로 회전 (90도)
    FVector FlankDirection = FVector::CrossProduct(ToTarget, FVector::UpVector);
    
    // 랜덤하게 좌측 또는 우측 선택
    if (FMath::RandBool())
    {
        FlankDirection = -FlankDirection;
    }
    
    return TargetLocation + (FlankDirection * FlankingDistance);
}

// 행동 오버라이드
void ATerminidWarrior::ProcessIdleBehavior(float DeltaTime)
{
    Super::ProcessIdleBehavior(DeltaTime);
    
    UpdateSwarmLeadership(DeltaTime);
    
    // 리더 워리어는 주변 순찰을 더 적극적으로 함
    if (bIsSwarmLeader && SwarmMemberCount > 0)
    {
        // 군집 형성 중심점 계산 및 이동
        CalculateSwarmFormation();
        MoveTowardsLocation(SwarmFormationCenter, DeltaTime);
    }
}

void ATerminidWarrior::ProcessPatrolBehavior(float DeltaTime)
{
    Super::ProcessPatrolBehavior(DeltaTime);
    
    UpdateSwarmLeadership(DeltaTime);
    UpdateTacticalMovement(DeltaTime);
}

void ATerminidWarrior::ProcessChaseBehavior(float DeltaTime)
{
    // 광폭화 상태 업데이트
    UpdateBerserkState(DeltaTime);
    
    // 군집 리더십 업데이트
    UpdateSwarmLeadership(DeltaTime);
    
    // 전술적 이동 사용
    if (bUseTacticalMovement && HasValidTarget())
    {
        ExecuteFlankingManeuver(DeltaTime);
    }
    else
    {
        Super::ProcessChaseBehavior(DeltaTime);
    }
    
    // 군집이 형성된 경우 군집 공격으로 전환
    if (bIsSwarmLeader && HasOptimalSwarmSize())
    {
        StateMachine->ChangeState(ETerminidState::Swarm);
    }
}

void ATerminidWarrior::ProcessAttackBehavior(float DeltaTime)
{
    UpdateBerserkState(DeltaTime);
    UpdateSwarmLeadership(DeltaTime);
    
    Super::ProcessAttackBehavior(DeltaTime);
    
    // 리더인 경우 주기적으로 군집 공격 조정
    if (bIsSwarmLeader)
    {
        CoordinateSwarmAttack();
    }
}

void ATerminidWarrior::ProcessSwarmBehavior(float DeltaTime)
{
    UpdateBerserkState(DeltaTime);
    UpdateSwarmLeadership(DeltaTime);
    
    if (bIsSwarmLeader)
    {
        // 리더로서 군집 조정
        ExecuteCoordinatedAttack(DeltaTime);
        IssueSwarmCommands();
    }
    else
    {
        // 일반 군집 행동
        Super::ProcessSwarmBehavior(DeltaTime);
    }
}

// 전투 오버라이드
bool ATerminidWarrior::CanPerformAttack() const
{
    return Super::CanPerformAttack();
}

void ATerminidWarrior::PerformAttack()
{
    if (!CanPerformAttack())
        return;
    
    // 광폭화 상태에서 공격력 증가
    float AttackMultiplier = bIsBerserk ? BerserkAttackMultiplier : 1.0f;
    
    // 리더십 보너스 적용
    if (bIsSwarmLeader && HasOptimalSwarmSize())
    {
        AttackMultiplier *= 1.1f; // 추가 10% 보너스
    }
    
    Super::PerformAttack();
}

void ATerminidWarrior::Die()
{
    // 군집 리더가 죽을 때 리더십 이양
    if (bIsSwarmLeader)
    {
        // 가장 가까운 워리어에게 리더십 이양
        ATerminidWarrior* NewLeader = nullptr;
        float ClosestDistance = FLT_MAX;
        
        for (ATerminidBase* Member : SwarmMembers)
        {
            ATerminidWarrior* WarriorMember = Cast<ATerminidWarrior>(Member);
            if (WarriorMember && WarriorMember->IsAlive() && WarriorMember->bCanLeadSwarm)
            {
                float Distance = FVector::Dist(GetActorLocation(), WarriorMember->GetActorLocation());
                if (Distance < ClosestDistance)
                {
                    ClosestDistance = Distance;
                    NewLeader = WarriorMember;
                }
            }
        }
        
        if (NewLeader)
        {
            NewLeader->BecomeSwarmLeader();
        }
    }
    
    // 마지막 광폭화 공격 (죽기 직전)
    if (!bIsBerserk && HasValidTarget())
    {
        EnterBerserkMode();
        if (IsTargetInAttackRange())
        {
            PerformAttack();
        }
    }
    
    Super::Die();
}

// 내부 함수들
void ATerminidWarrior::UpdateBerserkState(float DeltaTime)
{
    bool ShouldBeBerserk = GetHealthPercent() <= BerserkHealthThreshold;
    
    if (ShouldBeBerserk && !bIsBerserk)
    {
        EnterBerserkMode();
    }
    else if (!ShouldBeBerserk && bIsBerserk)
    {
        // 최소 지속 시간 체크
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - BerserkActivationTime >= BERSERK_MIN_DURATION)
        {
            ExitBerserkMode();
        }
    }
}

void ATerminidWarrior::UpdateSwarmLeadership(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if (CurrentTime - LastSwarmCheckTime >= SWARM_CHECK_INTERVAL)
    {
        LastSwarmCheckTime = CurrentTime;
        CheckForSwarmMembers();
        
        // 리더십 효과 적용
        if (CurrentTime - LastLeadershipUpdateTime >= LEADERSHIP_UPDATE_INTERVAL)
        {
            LastLeadershipUpdateTime = CurrentTime;
            BoostNearbyTerminids();
        }
    }
}

void ATerminidWarrior::UpdateTacticalMovement(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if (CurrentTime - LastTacticalUpdateTime >= TACTICAL_UPDATE_INTERVAL)
    {
        LastTacticalUpdateTime = CurrentTime;
        
        // 전술적 위치 재계산
        if (HasValidTarget() && bUseTacticalMovement)
        {
            FlankingTarget = CalculateFlankingPosition(CurrentTarget);
        }
    }
}

void ATerminidWarrior::CheckForSwarmMembers()
{
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    SwarmMembers.Empty();
    SwarmMemberCount = 0;
    
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ATerminidBase::StaticClass(), FoundActors);
    
    FVector MyLocation = GetActorLocation();
    
    for (AActor* Actor : FoundActors)
    {
        ATerminidBase* OtherTerminid = Cast<ATerminidBase>(Actor);
        if (OtherTerminid && OtherTerminid != this && OtherTerminid->IsAlive())
        {
            float Distance = FVector::Dist(MyLocation, OtherTerminid->GetActorLocation());
            if (Distance <= SwarmCoordinationRadius)
            {
                SwarmMembers.Add(OtherTerminid);
                SwarmMemberCount++;
            }
        }
    }
    
    // 군집이 형성되면 자동으로 리더가 됨
    if (!bIsSwarmLeader && bCanLeadSwarm && SwarmMemberCount >= OptimalSwarmSize / 2)
    {
        BecomeSwarmLeader();
    }
}

void ATerminidWarrior::IssueSwarmCommands()
{
    if (!bIsSwarmLeader)
        return;
    
    // 군집 멤버들에게 명령 전달
    for (ATerminidBase* Member : SwarmMembers)
    {
        if (IsValid(Member) && Member->IsAlive())
        {
            // 타겟이 없는 멤버에게 타겟 공유
            if (!Member->HasValidTarget() && HasValidTarget())
            {
                Member->SetCurrentTarget(CurrentTarget);
            }
            
            // 군집 상태로 전환
            if (Member->StateMachine && !Member->StateMachine->IsInState(ETerminidState::Swarm))
            {
                Member->StateMachine->ChangeState(ETerminidState::Swarm);
            }
        }
    }
}

void ATerminidWarrior::CalculateSwarmFormation()
{
    if (SwarmMembers.Num() == 0)
    {
        SwarmFormationCenter = GetActorLocation();
        return;
    }
    
    FVector CenterAccumulator = GetActorLocation();
    int32 ValidMemberCount = 1;
    
    for (ATerminidBase* Member : SwarmMembers)
    {
        if (IsValid(Member))
        {
            CenterAccumulator += Member->GetActorLocation();
            ValidMemberCount++;
        }
    }
    
    SwarmFormationCenter = CenterAccumulator / ValidMemberCount;
}

void ATerminidWarrior::ExecuteFlankingManeuver(float DeltaTime)
{
    if (!HasValidTarget())
        return;
    
    // 측면 공격 위치로 이동
    if (!bIsExecutingFlankManeuver)
    {
        bIsExecutingFlankManeuver = true;
        FlankingTarget = CalculateFlankingPosition(CurrentTarget);
    }
    
    float DistanceToFlankingPos = FVector::Dist(GetActorLocation(), FlankingTarget);
    
    if (DistanceToFlankingPos > 50.0f)
    {
        // 측면 위치로 이동
        MoveTowardsLocation(FlankingTarget, DeltaTime);
    }
    else
    {
        // 측면 위치에 도달하면 직접 공격
        bIsExecutingFlankManeuver = false;
        MoveTowardsTarget(DeltaTime);
    }
}

void ATerminidWarrior::ExecuteCoordinatedAttack(float DeltaTime)
{
    if (!HasValidTarget())
        return;
    
    // 군집 형성 확인 후 협력 공격
    CalculateSwarmFormation();
    
    // 리더는 중앙에서 지휘하며 직접 공격
    FVector TargetLocation = CurrentTarget->GetActorLocation();
    MoveTowardsLocation(TargetLocation, DeltaTime);
    
    // 주기적으로 군집 공격 조정
    CoordinateSwarmAttack();
}

void ATerminidWarrior::ApplyLeadershipBonus()
{
    // 리더십 보너스 로직 (필요시 구현)
}

void ATerminidWarrior::BoostNearbyTerminids()
{
    if (!bIsSwarmLeader)
        return;
    
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    // 리더십 범위 내의 다른 Terminid들에게 버프 효과
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ATerminidBase::StaticClass(), FoundActors);
    
    FVector MyLocation = GetActorLocation();
    
    for (AActor* Actor : FoundActors)
    {
        ATerminidBase* OtherTerminid = Cast<ATerminidBase>(Actor);
        if (OtherTerminid && OtherTerminid != this && OtherTerminid->IsAlive())
        {
            float Distance = FVector::Dist(MyLocation, OtherTerminid->GetActorLocation());
            if (Distance <= LeadershipRange)
            {
                // 스캐빈저들의 용기 증진 (도주 확률 감소)
                ATerminidScavenger* Scavenger = Cast<ATerminidScavenger>(OtherTerminid);
                if (Scavenger)
                {
                    // 리더십 효과로 스캐빈저의 도주 임계값 일시적 감소
                    // 실제 구현은 Scavenger 클래스에서 처리
                }
            }
        }
    }
}