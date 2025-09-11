#include "Huxley/ScavengerAnimInstance.h"
#include "Huxley/TerminidBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UScavengerAnimInstance::UScavengerAnimInstance()
{
	// 기본값 초기화
	TerminidOwner = nullptr;
	Speed = 0.0f;
	Direction = 0.0f;
	bIsMoving = false;
	bIsInAir = false;
	
	CurrentState = ETerminidState::Idle;
	bIsAlive = true;
	bIsBurrowed = false;
	bIsSpawning = false;
	
	bHasTarget = false;
	DistanceToTarget = 0.0f;
	bIsAttacking = false;
	bIsHurt = false;
	
	HealthPercent = 1.0f;
	bShouldFlee = false;
	
	LastFrameVelocity = FVector::ZeroVector;
	MovementThreshold = 10.0f; // 이동 감지 임계값
}

void UScavengerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	// Owner 캐시
	TerminidOwner = Cast<ATerminidBase>(TryGetPawnOwner());
}

void UScavengerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	if (!TerminidOwner)
	{
		return;
	}
	
	// 이동 정보 업데이트
	UCharacterMovementComponent* MovementComp = TerminidOwner->GetCharacterMovement();
	if (MovementComp)
	{
		FVector CurrentVelocity = MovementComp->Velocity;
		Speed = CurrentVelocity.Size2D();
		bIsMoving = Speed > MovementThreshold;
		bIsInAir = MovementComp->IsFalling();
		
		// 방향 계산 (캐릭터 기준 로컬 방향)
		if (bIsMoving)
		{
			FVector ForwardVector = TerminidOwner->GetActorForwardVector();
			FVector VelocityNormalized = CurrentVelocity.GetSafeNormal2D();
			
			// 내적과 외적을 이용한 방향 계산
			float ForwardDot = FVector::DotProduct(ForwardVector, VelocityNormalized);
			FVector CrossProduct = FVector::CrossProduct(ForwardVector, VelocityNormalized);
			Direction = UKismetMathLibrary::DegAtan2(CrossProduct.Z, ForwardDot);
		}
		else
		{
			Direction = 0.0f;
		}
	}
	
	// 상태 정보 업데이트
	if (TerminidOwner->StateMachine)
	{
		// FSM에서 현재 상태 가져오기 (FSM 구현에 따라 다를 수 있음)
		// 임시로 공개된 멤버들을 통해 상태 추론
		CurrentState = ETerminidState::Idle; // 기본값
		
		// 생존 상태
		bIsAlive = TerminidOwner->IsAlive();
		if (!bIsAlive)
		{
			CurrentState = ETerminidState::Death;
		}
	}
	
	// Burrow 상태
	bIsBurrowed = TerminidOwner->IsBurrowed();
	if (bIsBurrowed)
	{
		CurrentState = ETerminidState::Burrow;
	}
	
	// 스폰 상태
	bIsSpawning = TerminidOwner->IsSpawning();
	
	// 전투 관련 정보
	bHasTarget = TerminidOwner->HasValidTarget();
	DistanceToTarget = TerminidOwner->DistanceToTarget;
	
	// 공격 상태 (공격 범위 내에 타겟이 있을 때)
	bIsAttacking = bHasTarget && TerminidOwner->IsTargetInAttackRange() && bIsAlive;
	if (bIsAttacking)
	{
		CurrentState = ETerminidState::Attack;
	}
	else if (bHasTarget && bIsAlive && !bIsBurrowed)
	{
		CurrentState = ETerminidState::Chase;
	}
	
	// 체력 정보
	HealthPercent = TerminidOwner->GetHealthPercent();
	bShouldFlee = TerminidOwner->ShouldFlee();
	if (bShouldFlee && bIsAlive && !bIsBurrowed)
	{
		CurrentState = ETerminidState::Flee;
	}
	
	// Hurt 상태 감지 (체력이 감소했을 때의 임시적 상태)
	// 이는 FSM에서 직접 가져와야 하므로 현재는 근사치로 처리
	bIsHurt = false; // FSM에서 직접 상태를 확인해야 함
}

// 애니메이션 상태 확인 함수들
bool UScavengerAnimInstance::ShouldPlayIdleAnimation() const
{
	return CurrentState == ETerminidState::Idle && bIsAlive && !bIsMoving;
}

bool UScavengerAnimInstance::ShouldPlayMoveAnimation() const
{
	return (CurrentState == ETerminidState::Chase || 
	        CurrentState == ETerminidState::Patrol || 
	        CurrentState == ETerminidState::Flee) && 
	        bIsAlive && bIsMoving;
}

bool UScavengerAnimInstance::ShouldPlayAttackAnimation() const
{
	return CurrentState == ETerminidState::Attack && bIsAlive;
}

bool UScavengerAnimInstance::ShouldPlayHurtAnimation() const
{
	return CurrentState == ETerminidState::Hurt && bIsAlive;
}

bool UScavengerAnimInstance::ShouldPlayDeathAnimation() const
{
	return CurrentState == ETerminidState::Death && !bIsAlive;
}

bool UScavengerAnimInstance::ShouldPlayBurrowAnimation() const
{
	return CurrentState == ETerminidState::Burrow && bIsBurrowed;
}

bool UScavengerAnimInstance::ShouldPlayEmergeAnimation() const
{
	// 이머지 애니메이션은 Burrow에서 벗어날 때 재생
	// 이를 위해서는 이전 프레임 상태를 추적하거나 별도 플래그가 필요
	// 현재는 단순하게 !bIsBurrowed && 이전에 burrowed였던 상태로 근사
	return !bIsBurrowed && CurrentState == ETerminidState::Idle && bIsAlive;
}

float UScavengerAnimInstance::GetMovementPlayRate() const
{
	// 이동 속도에 따른 애니메이션 재생 속도 조절
	if (!bIsMoving || !TerminidOwner)
	{
		return 1.0f;
	}
	
	// 기본 이동 속도 대비 현재 속도 비율
	float BaseSpeed = 300.0f; // Scavenger 기본 속도
	if (TerminidOwner->BaseStats.MoveSpeed > 0.0f)
	{
		BaseSpeed = TerminidOwner->BaseStats.MoveSpeed;
	}
	
	float PlayRate = Speed / BaseSpeed;
	
	// 재생 속도를 합리적 범위로 제한
	return FMath::Clamp(PlayRate, 0.5f, 2.0f);
}