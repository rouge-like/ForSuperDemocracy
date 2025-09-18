#include "Huxley/TerminidAnimInstance.h"
#include "Huxley/TerminidBase.h"
#include "Huxley/TerminidFSM.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UTerminidAnimInstance::UTerminidAnimInstance()
{
    // 기본값 초기화
    bIsSpawning = false;
    bIsDead = false;
    bIsAttacking = false;
    bIsHurt = false;
    bIsFleeing = false;
    bIsAlive = true;
    bShouldFlee = false;
    
    Speed = 0.0f;
    Direction = 0.0f;
    bIsMoving = false;
    bShouldMove = true;
    bIsInAir = false;
    
    HealthPercent = 1.0f;
    bHasTarget = false;
    DistanceToTarget = 0.0f;
    
    CurrentState = ETerminidState::Idle;
    TerminidType = ETerminidType::Scavenger;
    
    TerminidOwner = nullptr;
    
    SpawnMontage = nullptr;
    AttackMontage = nullptr;
    DeathMontage = nullptr;
    HurtMontage = nullptr;
    
    // 비공개 변수들 초기화
    MovementThreshold = 10.0f;
}

void UTerminidAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    // Owner 설정
    TerminidOwner = Cast<ATerminidBase>(TryGetPawnOwner());
    
    if (TerminidOwner)
    {
        TerminidType = TerminidOwner->GetTerminidType();
    }
}

void UTerminidAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);
    
    if (!TerminidOwner)
    {
        TerminidOwner = Cast<ATerminidBase>(TryGetPawnOwner());
        return;
    }
    
    // 애니메이션 변수들 업데이트
    UpdateMovementVariables();
    UpdateCombatVariables();
    UpdateStateVariables();
}

void UTerminidAnimInstance::UpdateMovementVariables()
{
    if (!TerminidOwner)
        return;
    
    // 속도 계산
    FVector Velocity = TerminidOwner->GetVelocity();
    Speed = Velocity.Size2D(); // 2D 속도만 사용
    
    // 이동 여부
    bIsMoving = Speed > MovementThreshold;
    
    // 공중 상태 확인
    if (UCharacterMovementComponent* MovementComp = TerminidOwner->GetCharacterMovement())
    {
        bIsInAir = MovementComp->IsFalling();
    }
    
    // 방향 계산 (캐릭터의 forward vector와 velocity의 각도)
    if (bIsMoving)
    {
        FVector ForwardVector = TerminidOwner->GetActorForwardVector();
        FVector VelocityNormalized = Velocity.GetSafeNormal2D();
        
        // 내적과 외적을 이용한 방향 계산
        float ForwardDot = FVector::DotProduct(ForwardVector, VelocityNormalized);
        FVector CrossProduct = FVector::CrossProduct(ForwardVector, VelocityNormalized);
        Direction = FMath::RadiansToDegrees(FMath::Atan2(CrossProduct.Z, ForwardDot));
    }
    else
    {
        Direction = 0.0f;
    }
    
    // 이동 가능 여부 (스폰 중이거나 죽었으면 false)
    bShouldMove = !bIsSpawning && bIsAlive;
}

void UTerminidAnimInstance::UpdateCombatVariables()
{
    if (!TerminidOwner)
        return;
    
    // 체력 퍼센트
    HealthPercent = TerminidOwner->GetHealthPercent();
    
    // 타겟 정보
    bHasTarget = TerminidOwner->HasValidTarget();
    DistanceToTarget = TerminidOwner->DistanceToTarget;
    
    // 도망 여부
    bShouldFlee = TerminidOwner->ShouldFlee();
    
    // 공격 상태 (실제로 공격 가능할 때만 true)
    bIsAttacking = bHasTarget && TerminidOwner->CanPerformAttack() && bIsAlive;
    
    // 공격 애니메이션 재생 중 여부 (C++에서 관리)
    bIsPlayingAttackAnimation = TerminidOwner->IsPlayingAttackAnimation();
}

void UTerminidAnimInstance::UpdateStateVariables()
{
    if (!TerminidOwner)
        return;
    
    // 생존 상태
    bIsAlive = TerminidOwner->IsAlive();
    bIsDead = !bIsAlive;
    
    // 스폰 상태
    bIsSpawning = TerminidOwner->IsSpawning();
    
    // FSM 상태
    if (TerminidOwner->StateMachine)
    {
        CurrentState = TerminidOwner->StateMachine->GetCurrentState();
        
        // 상태별 플래그 업데이트
        bIsHurt = (CurrentState == ETerminidState::Hurt);
        bIsFleeing = (CurrentState == ETerminidState::Flee);
    }
    else
    {
        // FSM이 없을 때 기본 상태 추론
        if (!bIsAlive)
        {
            CurrentState = ETerminidState::Death;
        }
        else if (bIsAttacking)
        {
            CurrentState = ETerminidState::Attack;
        }
        else if (bShouldFlee)
        {
            CurrentState = ETerminidState::Flee;
        }
        else if (bHasTarget && bIsAlive)
        {
            CurrentState = ETerminidState::Chase;
        }
        else
        {
            CurrentState = ETerminidState::Idle;
        }
    }
}

// 애니메이션 상태 확인 함수들 (기본 구현)
bool UTerminidAnimInstance::ShouldPlayIdleAnimation() const
{
    return CurrentState == ETerminidState::Idle && bIsAlive && !bIsMoving;
}

bool UTerminidAnimInstance::ShouldPlayMoveAnimation() const
{
    return (CurrentState == ETerminidState::Chase ||
            CurrentState == ETerminidState::Patrol ||
            CurrentState == ETerminidState::Flee) &&
            bIsAlive && bIsMoving;
}

bool UTerminidAnimInstance::ShouldPlayAttackAnimation() const
{
    return CurrentState == ETerminidState::Attack && bIsAlive;
}

bool UTerminidAnimInstance::ShouldPlayHurtAnimation() const
{
    return CurrentState == ETerminidState::Hurt && bIsAlive;
}

bool UTerminidAnimInstance::ShouldPlayDeathAnimation() const
{
    return CurrentState == ETerminidState::Death || !bIsAlive;
}


float UTerminidAnimInstance::GetMovementPlayRate() const
{
    // 이동 속도에 따른 애니메이션 재생 속도 조절
    if (!bIsMoving || !TerminidOwner)
    {
        return 1.0f;
    }
    
    // 기본 이동 속도 대비 현재 속도 비율
    float BaseSpeed = 300.0f;
    if (TerminidOwner->BaseStats.MoveSpeed > 0.0f)
    {
        BaseSpeed = TerminidOwner->BaseStats.MoveSpeed;
    }
    
    float PlayRate = Speed / BaseSpeed;
    
    // 재생 속도를 합리적 범위로 제한
    return FMath::Clamp(PlayRate, 0.5f, 2.0f);
}

#if WITH_EDITOR
void UTerminidAnimInstance::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    
    // 에디터에서 속성 변경 시 처리
    if (PropertyChangedEvent.Property)
    {
        // 필요시 추가 처리
    }
}
#endif