#include "Huxley/TerminidScavenger.h"
#include "Huxley/TerminidFSM.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

ATerminidScavenger::ATerminidScavenger()
{
    // 스캐빈저 기본 설정
    FleeHealthThreshold = 0.3f;
    FleeDistance = 500.0f;
    AggressionBoost = 1.2f;
    GroupDetectionRadius = 200.0f;
    bCanCallForHelp = true;
    HelpCallRadius = 400.0f;
    
    // 내부 상태 초기화
    bIsInGroup = false;
    bHasCalledForHelp = false;
    LastGroupCheckTime = 0.0f;
    NearbyScavengerCount = 0;
    FleeTargetLocation = FVector::ZeroVector;
    bIsFleeingActive = false;
    FleeStartTime = 0.0f;
    LastHelpCallTime = 0.0f;
}

void ATerminidScavenger::BeginPlay()
{
    Super::BeginPlay();
    
    // 스캐빈저 전용 스탯으로 초기화
    FTerminidStats ScavengerStats = FTerminidStats::CreateScavengerStats();
    InitializeTerminid(ScavengerStats, ETerminidType::Scavenger);
    
    // 체력 초기화는 InitializeTerminid에서 처리됨
}

// 상태 확인 함수들
bool ATerminidScavenger::ShouldFleeFromCombat() const
{
    return GetHealthPercent() <= FleeHealthThreshold;
}

bool ATerminidScavenger::IsInGroup() const
{
    return bIsInGroup && (NearbyScavengerCount > 0);
}

int32 ATerminidScavenger::GetNearbyScavengerCount() const
{
    return NearbyScavengerCount;
}

void ATerminidScavenger::CallForHelp()
{
    // 간단한 도움 요청 - 나중에 확장 가능
    bHasCalledForHelp = true;
}

void ATerminidScavenger::RespondToHelpCall(ATerminidScavenger* Caller)
{
    // 간단한 응답 - 나중에 확장 가능
}

// 행동 오버라이드
void ATerminidScavenger::ProcessIdleBehavior(float DeltaTime)
{
    Super::ProcessIdleBehavior(DeltaTime);
    
    // 스캐빈저 특화 대기 행동
    // 1. 주위를 경계하며 작은 움직임
    // 2. 애니메이션 이벤트 호출
    OnIdleAnimation();
    
    // 가끔씩 작은 랜덤 이동 (더 생동감 있게)
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastGroupCheckTime > 3.0f) // 3초마다
    {
        LastGroupCheckTime = CurrentTime;
        
        // 랜덤한 작은 이동
        FVector RandomOffset = FVector(
            FMath::RandRange(-100.0f, 100.0f),
            FMath::RandRange(-100.0f, 100.0f),
            0.0f
        );
        
        FVector TargetLocation = GetActorLocation() + RandomOffset;
        MoveTowardsLocation(TargetLocation, DeltaTime);
    }
}

void ATerminidScavenger::ProcessChaseBehavior(float DeltaTime)
{
    // 체력 확인 후 도주 결정
    if (ShouldFleeFromCombat())
    {
        StateMachine->ChangeState(ETerminidState::Flee);
        return;
    }
    
    // 스캐빈저는 빠르고 민첩한 움직임 - 이동 애니메이션 호출
    OnMoveAnimation();
    
    // 기본 추적 행동 (더 빠른 속도로)
    Super::ProcessChaseBehavior(DeltaTime);
    
    // 스캐빈저의 특징: 지그재그 움직임으로 더 생동감 있게
    if (HasValidTarget())
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        
        // 0.5초마다 약간의 지그재그 움직임 추가
        if (FMath::Fmod(CurrentTime, 1.0f) < DeltaTime)
        {
            FVector SideStep = GetActorRightVector() * FMath::RandRange(-50.0f, 50.0f);
            FVector CurrentLocation = GetActorLocation();
            MoveTowardsLocation(CurrentLocation + SideStep, DeltaTime);
        }
    }
}

void ATerminidScavenger::ProcessAttackBehavior(float DeltaTime)
{
    // 체력 확인 후 도주 결정
    if (ShouldFleeFromCombat())
    {
        StateMachine->ChangeState(ETerminidState::Flee);
        return;
    }
    
    // 공격 애니메이션 호출
    OnAttackAnimation();
    
    // 기본 공격 행동
    Super::ProcessAttackBehavior(DeltaTime);
}

void ATerminidScavenger::ProcessFleeBehavior(float DeltaTime)
{
    // 간단한 도주 - 플레이어 반대 방향으로 이동
    if (HasValidTarget())
    {
        FVector FleeDirection = (GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal();
        FVector FleeTarget = GetActorLocation() + (FleeDirection * 500.0f);
        MoveTowardsLocation(FleeTarget, DeltaTime);
    }
}

void ATerminidScavenger::ProcessSwarmBehavior(float DeltaTime)
{
    // 간단한 군집 - 일반 추적과 동일하게 처리
    StateMachine->ChangeState(ETerminidState::Chase);
}

// 전투 오버라이드
bool ATerminidScavenger::CanPerformAttack() const
{
    // 기본 공격 가능 조건 확인
    if (!Super::CanPerformAttack())
        return false;
    
    // 도주 상태에서는 공격하지 않음
    if (ShouldFleeFromCombat())
        return false;
    
    return true;
}

void ATerminidScavenger::PerformAttack()
{
    if (!CanPerformAttack())
        return;
    
    // 기본 공격 수행
    Super::PerformAttack();
}

void ATerminidScavenger::Die()
{
    // 죽음 애니메이션 호출
    OnDeathAnimation();
    
    // 간단한 죽음 처리
    Super::Die();
}

// 간단한 내부 함수들
void ATerminidScavenger::UpdateFleeLogic(float DeltaTime)
{
    // 간단한 도주 로직은 ProcessFleeBehavior에서 처리
}

void ATerminidScavenger::UpdateGroupBehavior(float DeltaTime)
{
    // 그룹 기능은 나중에 확장 예정
}

void ATerminidScavenger::CheckForNearbyScavengers()
{
    // 간단한 그룹 감지 - 나중에 확장 예정
    bIsInGroup = false;
    NearbyScavengerCount = 0;
}

void ATerminidScavenger::ProcessQuickMovement(const FVector& TargetLocation, float DeltaTime)
{
    // 기본 이동 사용
    MoveTowardsLocation(TargetLocation, DeltaTime);
}

float ATerminidScavenger::GetGroupAttackMultiplier() const
{
    // 기본 공격력 (그룹 보너스 없음)
    return 1.0f;
}