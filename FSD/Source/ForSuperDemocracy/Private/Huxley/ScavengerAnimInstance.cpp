#include "Huxley/ScavengerAnimInstance.h"
#include "Huxley/TerminidScavenger.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"

UScavengerAnimInstance::UScavengerAnimInstance()
{
	// Scavenger 전용 변수 초기화
	ScavengerOwner = nullptr;
	AttackChargePercent = 0.0f;
	bIsAttackCharging = false;
	TimeSinceLastAttack = 0.0f;
	
	LastAttackTime = 0.0f;
}

void UScavengerAnimInstance::NativeInitializeAnimation()
{
	// 부모 클래스의 초기화 먼저 호출
	Super::NativeInitializeAnimation();
	
	// Scavenger Owner 캐시 (부모의 TerminidOwner를 Scavenger로 캐스팅)
	ScavengerOwner = Cast<ATerminidScavenger>(TerminidOwner);
}

void UScavengerAnimInstance::UpdateMovementVariables()
{
	// 부모 클래스의 기본 이동 변수 업데이트
	Super::UpdateMovementVariables();
	
	// Scavenger 고유 이동 관련 업데이트는 여기에 추가
	// 현재는 부모 클래스 구현으로 충분
}

void UScavengerAnimInstance::UpdateCombatVariables()
{
	// 부모 클래스의 기본 전투 변수 업데이트
	Super::UpdateCombatVariables();
	
	if (!ScavengerOwner)
		return;
		
	// Scavenger 고유 전투 관련 변수 업데이트 (간단한 버전)
	
	// 공격 차징 상태 확인 - 기본적으로 공격 중이면 차징으로 간주
	bIsAttackCharging = bIsAttacking;
	AttackChargePercent = bIsAttackCharging ? 1.0f : 0.0f;
	
	// 마지막 공격 이후 시간 계산
	if (UWorld* World = GetWorld())
	{
		float CurrentTime = World->GetTimeSeconds();
		
		if (bIsAttacking && LastAttackTime == 0.0f)
		{
			// 공격이 시작되면 시간 기록
			LastAttackTime = CurrentTime;
			TimeSinceLastAttack = 0.0f;
		}
		else if (!bIsAttacking && LastAttackTime > 0.0f)
		{
			// 공격이 끝났으면 시간 계산 시작
			TimeSinceLastAttack = CurrentTime - LastAttackTime;
			LastAttackTime = 0.0f; // 리셋
		}
	}
}

void UScavengerAnimInstance::UpdateStateVariables()
{
	// 부모 클래스의 기본 상태 변수 업데이트
	Super::UpdateStateVariables();
	
	// Scavenger 고유 상태 관련 업데이트는 현재 필요 없음
	// 필요한 경우 여기에 추가
}

float UScavengerAnimInstance::GetMovementPlayRate() const
{
	if (!ScavengerOwner)
		return Super::GetMovementPlayRate();
	
	// Scavenger 고유 이동 속도 계산 로직
	float BasePlayRate = Super::GetMovementPlayRate();
	
	// Scavenger가 도망가는 중이면 애니메이션 속도 증가
	if (bIsFleeing)
	{
		BasePlayRate *= 1.3f; // 30% 빠르게
	}
	
	// 공격 차징 중이면 느리게
	if (bIsAttackCharging)
	{
		BasePlayRate *= 0.7f; // 30% 느리게
	}
	
	return FMath::Clamp(BasePlayRate, 0.3f, 2.5f);
}

// Scavenger 전용 유틸리티 함수들
bool UScavengerAnimInstance::ShouldPlayScavengerIdleVariation() const
{
	if (!ScavengerOwner || !ShouldPlayIdleAnimation())
		return false;
	
	// Idle 상태에서 가끔씩 다양한 애니메이션 재생
	// 체력이 낮거나 긴장 상태일 때 다른 Idle 사용
	return HealthPercent < 0.3f || bHasTarget;
}

bool UScavengerAnimInstance::ShouldPlayScavengerAttackCombo() const
{
	if (!ScavengerOwner || !bIsAttacking)
		return false;
	
	// 연속 공격 조건: 최근에 공격했고, 타겟이 가까이 있을 때
	return TimeSinceLastAttack < 2.0f && DistanceToTarget < 150.0f && bHasTarget;
}