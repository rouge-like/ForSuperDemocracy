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
    
    Speed = 0.0f;
    Direction = 0.0f;
    bIsMoving = false;
    bShouldMove = true;
    
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
    Speed = Velocity.Size();
    
    // 이동 여부
    bIsMoving = Speed > 3.0f;
    
    // 방향 계산 (캐릭터의 forward vector와 velocity의 각도)
    if (Speed > 0.0f)
    {
        FVector ForwardVector = TerminidOwner->GetActorForwardVector();
        FVector VelocityNormalized = Velocity.GetSafeNormal();
        
        float DotProduct = FVector::DotProduct(ForwardVector, VelocityNormalized);
        Direction = FMath::RadiansToDegrees(FMath::Acos(DotProduct));
        
        // 좌우 구분
        FVector CrossProduct = FVector::CrossProduct(ForwardVector, VelocityNormalized);
        if (CrossProduct.Z < 0.0f)
        {
            Direction = -Direction;
        }
    }
    else
    {
        Direction = 0.0f;
    }
    
    // 이동 가능 여부 (스폰 중이거나 죽었으면 false)
    bShouldMove = !bIsSpawning && !bIsDead;
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
}

void UTerminidAnimInstance::UpdateStateVariables()
{
    if (!TerminidOwner)
        return;
    
    // 스폰 상태
    bIsSpawning = TerminidOwner->IsSpawning();
    
    // 죽음 상태
    bIsDead = !TerminidOwner->IsAlive();
    
    // FSM 상태
    if (TerminidOwner->StateMachine)
    {
        CurrentState = TerminidOwner->StateMachine->GetCurrentState();
        
        // 상태별 플래그 업데이트
        bIsAttacking = (CurrentState == ETerminidState::Attack);
        bIsHurt = (CurrentState == ETerminidState::Hurt);
        bIsFleeing = (CurrentState == ETerminidState::Flee);
    }
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