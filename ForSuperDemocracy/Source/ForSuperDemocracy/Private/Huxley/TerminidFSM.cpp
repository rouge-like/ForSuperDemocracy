#include "Huxley/TerminidFSM.h"
#include "Huxley/TerminidBase.h"
#include "Engine/World.h"
#include "TimerManager.h"

UTerminidFSM::UTerminidFSM()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // 기본값 초기화
    CurrentState = ETerminidState::Idle;
    PreviousState = ETerminidState::Idle;
    CurrentStateTime = 0.0f;
    bCanChangeState = true;
    bIsInitialized = false;
    bStateChangeLocked = false;
    OwnerTerminid = nullptr;
}

void UTerminidFSM::BeginPlay()
{
    Super::BeginPlay();
    
    // 소유자 Terminid 참조 저장
    OwnerTerminid = Cast<ATerminidBase>(GetOwner());
    
    if (OwnerTerminid)
    {
        bIsInitialized = true;
        // 초기 상태 진입
        OnEnterState(CurrentState);
    }
}

void UTerminidFSM::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // 초기화되지 않았으면 실행하지 않음
    if (!bIsInitialized)
        return;
        
    UpdateStateMachine(DeltaTime);
}

void UTerminidFSM::UpdateStateMachine(float DeltaTime)
{
    if (!OwnerTerminid || !bIsInitialized)
        return;
    
    // 현재 상태 시간 업데이트
    CurrentStateTime += DeltaTime;
    
    // 현재 상태에 따른 처리
    switch (CurrentState)
    {
        case ETerminidState::Idle:
            ProcessIdleState(DeltaTime);
            break;
            
        case ETerminidState::Patrol:
            ProcessPatrolState(DeltaTime);
            break;
            
        case ETerminidState::Chase:
            ProcessChaseState(DeltaTime);
            break;
            
        case ETerminidState::Attack:
            ProcessAttackState(DeltaTime);
            break;
            
        case ETerminidState::Hurt:
            ProcessHurtState(DeltaTime);
            break;
            
        case ETerminidState::Death:
            ProcessDeathState(DeltaTime);
            break;
            
        case ETerminidState::Swarm:
            ProcessSwarmState(DeltaTime);
            break;
            
        case ETerminidState::Flee:
            ProcessFleeState(DeltaTime);
            break;
    }
}

void UTerminidFSM::ChangeState(ETerminidState NewState)
{
    // 상태 변경 가능 여부 확인
    if (!bCanChangeState || bStateChangeLocked)
        return;
    
    // 현재 상태와 같으면 무시
    if (CurrentState == NewState)
        return;
    
    // 상태 변경 유효성 검사
    if (!CanTransitionTo(NewState))
        return;
    
    // 상태 변경 실행
    ETerminidState OldState = CurrentState;
    
    OnExitState(OldState);
    
    PreviousState = CurrentState;
    CurrentState = NewState;
    CurrentStateTime = 0.0f;
    
    OnEnterState(NewState);
}

void UTerminidFSM::ForceChangeState(ETerminidState NewState)
{
    // 현재 상태와 같으면 무시
    if (CurrentState == NewState)
        return;
    
    // 강제 상태 변경 (제한 무시)
    ETerminidState OldState = CurrentState;
    
    OnExitState(OldState);
    
    PreviousState = CurrentState;
    CurrentState = NewState;
    CurrentStateTime = 0.0f;
    
    OnEnterState(NewState);
}

void UTerminidFSM::RevertToPreviousState()
{
    if (PreviousState != CurrentState)
    {
        ChangeState(PreviousState);
    }
}

// 상태 그룹 확인 함수들
bool UTerminidFSM::IsInCombatState() const
{
    return CurrentState == ETerminidState::Attack || 
           CurrentState == ETerminidState::Chase ||
           CurrentState == ETerminidState::Swarm;
}

bool UTerminidFSM::IsInMovementState() const
{
    return CurrentState == ETerminidState::Patrol ||
           CurrentState == ETerminidState::Chase ||
           CurrentState == ETerminidState::Swarm ||
           CurrentState == ETerminidState::Flee;
}

bool UTerminidFSM::IsInPassiveState() const
{
    return CurrentState == ETerminidState::Idle ||
           CurrentState == ETerminidState::Patrol;
}

void UTerminidFSM::SetStateChangeLock(bool bLocked)
{
    bStateChangeLocked = bLocked;
}

void UTerminidFSM::ChangeStateAfterTime(ETerminidState NewState, float Delay)
{
    if (Delay <= 0.0f)
    {
        ChangeState(NewState);
        return;
    }
    
    GetWorld()->GetTimerManager().SetTimer(
        StateChangeTimerHandle,
        [this, NewState]() { ChangeState(NewState); },
        Delay,
        false
    );
}

void UTerminidFSM::EnterHurtState(float HurtDuration)
{
    ForceChangeState(ETerminidState::Hurt);
    
    // 일정 시간 후 이전 상태로 복원
    GetWorld()->GetTimerManager().SetTimer(
        HurtRecoveryTimerHandle,
        [this]() { RevertToPreviousState(); },
        HurtDuration,
        false
    );
}

// 상태별 처리 함수들
void UTerminidFSM::ProcessIdleState(float DeltaTime)
{
    OwnerTerminid->ProcessIdleBehavior(DeltaTime);
    
    // 타겟이 있으면 추적 상태로
    if (OwnerTerminid->HasValidTarget())
    {
        ChangeState(ETerminidState::Chase);
    }
}

void UTerminidFSM::ProcessPatrolState(float DeltaTime)
{
    OwnerTerminid->ProcessPatrolBehavior(DeltaTime);
    
    // 타겟이 있으면 추적 상태로
    if (OwnerTerminid->HasValidTarget())
    {
        ChangeState(ETerminidState::Chase);
    }
}

void UTerminidFSM::ProcessChaseState(float DeltaTime)
{
    OwnerTerminid->ProcessChaseBehavior(DeltaTime);
    
    // 타겟이 없으면 대기 상태로
    if (!OwnerTerminid->HasValidTarget())
    {
        ChangeState(ETerminidState::Idle);
        return;
    }
    
    // 공격 범위 안에 있으면 공격 상태로
    if (OwnerTerminid->IsTargetInAttackRange())
    {
        ChangeState(ETerminidState::Attack);
        return;
    }
    
    // 체력이 낮으면 도주 상태로
    if (OwnerTerminid->ShouldFlee())
    {
        ChangeState(ETerminidState::Flee);
    }
}

void UTerminidFSM::ProcessAttackState(float DeltaTime)
{
    OwnerTerminid->ProcessAttackBehavior(DeltaTime);
    
    // 타겟이 없으면 대기 상태로
    if (!OwnerTerminid->HasValidTarget())
    {
        ChangeState(ETerminidState::Idle);
        return;
    }
    
    // 공격 범위를 벗어나면 추적 상태로
    if (!OwnerTerminid->IsTargetInAttackRange())
    {
        ChangeState(ETerminidState::Chase);
        return;
    }
    
    // 체력이 낮으면 도주 상태로
    if (OwnerTerminid->ShouldFlee())
    {
        ChangeState(ETerminidState::Flee);
    }
}

void UTerminidFSM::ProcessHurtState(float DeltaTime)
{
    OwnerTerminid->ProcessHurtBehavior(DeltaTime);
    
    // Hurt 상태는 타이머로 자동 복원되므로 여기서는 특별한 처리 없음
}

void UTerminidFSM::ProcessDeathState(float DeltaTime)
{
    OwnerTerminid->ProcessDeathBehavior(DeltaTime);
    
    // 죽음 상태에서는 다른 상태로 변경되지 않음
}

void UTerminidFSM::ProcessSwarmState(float DeltaTime)
{
    OwnerTerminid->ProcessSwarmBehavior(DeltaTime);
    
    // 타겟이 없으면 대기 상태로
    if (!OwnerTerminid->HasValidTarget())
    {
        ChangeState(ETerminidState::Idle);
        return;
    }
    
    // 공격 범위 안에 있으면 공격 상태로
    if (OwnerTerminid->IsTargetInAttackRange())
    {
        ChangeState(ETerminidState::Attack);
        return;
    }
}

void UTerminidFSM::ProcessFleeState(float DeltaTime)
{
    OwnerTerminid->ProcessFleeBehavior(DeltaTime);
    
    // 체력이 회복되면 추적 상태로 (체력 50% 이상)
    if (OwnerTerminid->GetHealthPercent() > 0.5f && OwnerTerminid->HasValidTarget())
    {
        ChangeState(ETerminidState::Chase);
    }
    // 타겟이 없으면 대기 상태로
    else if (!OwnerTerminid->HasValidTarget())
    {
        ChangeState(ETerminidState::Idle);
    }
}

// 상태 진입/종료 함수들
void UTerminidFSM::OnEnterState(ETerminidState NewState)
{
    switch (NewState)
    {
        case ETerminidState::Idle:
            OnEnterIdleState();
            break;
        case ETerminidState::Patrol:
            OnEnterPatrolState();
            break;
        case ETerminidState::Chase:
            OnEnterChaseState();
            break;
        case ETerminidState::Attack:
            OnEnterAttackState();
            break;
        case ETerminidState::Hurt:
            OnEnterHurtState();
            break;
        case ETerminidState::Death:
            OnEnterDeathState();
            break;
        case ETerminidState::Swarm:
            OnEnterSwarmState();
            break;
        case ETerminidState::Flee:
            OnEnterFleeState();
            break;
    }
}

void UTerminidFSM::OnExitState(ETerminidState OldState)
{
    // 타이머 정리
    GetWorld()->GetTimerManager().ClearTimer(StateChangeTimerHandle);
    
    switch (OldState)
    {
        case ETerminidState::Idle:
            OnExitIdleState();
            break;
        case ETerminidState::Patrol:
            OnExitPatrolState();
            break;
        case ETerminidState::Chase:
            OnExitChaseState();
            break;
        case ETerminidState::Attack:
            OnExitAttackState();
            break;
        case ETerminidState::Hurt:
            OnExitHurtState();
            break;
        case ETerminidState::Death:
            OnExitDeathState();
            break;
        case ETerminidState::Swarm:
            OnExitSwarmState();
            break;
        case ETerminidState::Flee:
            OnExitFleeState();
            break;
    }
}

// 상태별 진입 함수들
void UTerminidFSM::OnEnterIdleState()
{
    // 대기 상태 진입 시 이동 정지
    if (OwnerTerminid)
    {
        OwnerTerminid->StopMovement();
    }
}

void UTerminidFSM::OnEnterPatrolState()
{
    // 순찰 상태 진입 시 처리
}

void UTerminidFSM::OnEnterChaseState()
{
    // 추적 상태 진입 시 처리 - 별도 설정 불필요
}

void UTerminidFSM::OnEnterAttackState()
{
    // 공격 상태 진입 시 처리
}

void UTerminidFSM::OnEnterHurtState()
{
    // 피격 상태 진입 시 처리
    SetStateChangeLock(true); // 피격 중에는 상태 변경 제한
}

void UTerminidFSM::OnEnterDeathState()
{
    // 죽음 상태 진입 시 처리
    SetStateChangeLock(true); // 죽음 후에는 상태 변경 불가
}

void UTerminidFSM::OnEnterSwarmState()
{
    // 군집 상태 진입 시 처리
}

void UTerminidFSM::OnEnterFleeState()
{
    // 도주 상태 진입 시 처리
}

// 상태별 종료 함수들
void UTerminidFSM::OnExitIdleState()
{
    // 대기 상태 종료 시 처리
}

void UTerminidFSM::OnExitPatrolState()
{
    // 순찰 상태 종료 시 처리
}

void UTerminidFSM::OnExitChaseState()
{
    // 추적 상태 종료 시 처리
}

void UTerminidFSM::OnExitAttackState()
{
    // 공격 상태 종료 시 처리
}

void UTerminidFSM::OnExitHurtState()
{
    // 피격 상태 종료 시 처리
    SetStateChangeLock(false); // 피격 상태 종료 후 상태 변경 허용
    GetWorld()->GetTimerManager().ClearTimer(HurtRecoveryTimerHandle);
}

void UTerminidFSM::OnExitDeathState()
{
    // 죽음 상태 종료 시 처리 (실제로는 호출되지 않음)
}

void UTerminidFSM::OnExitSwarmState()
{
    // 군집 상태 종료 시 처리
}

void UTerminidFSM::OnExitFleeState()
{
    // 도주 상태 종료 시 처리
}

// 상태 변경 유효성 검사
bool UTerminidFSM::CanTransitionTo(ETerminidState NewState) const
{
    // 죽음 상태에서는 다른 상태로 변경 불가
    if (CurrentState == ETerminidState::Death)
        return false;
    
    // 죽음 상태로는 언제든지 변경 가능
    if (NewState == ETerminidState::Death)
        return true;
    
    // 기본적으로 모든 상태 변경 허용
    return true;
}