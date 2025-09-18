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

	// Idle 상태 랜덤 이동 초기화
	IdleTargetLocation = FVector::ZeroVector;
	bHasIdleTarget = false;
	IdleTargetReachThreshold = 60.0f; // 60유닛 이내에 도달하면 새 목표 설정 (더 정확한 도달 판정)

	// 스캐빈저는 더 빠른 회복 (더 민첩한 유닛)
	HurtRecoveryTime = 0.5f; // 기본 1.5초 -> 0.5초로 단축

	// 스캐빈저 특화 감지 시스템 (매우 넓은 감지 범위)
	SightAngle = 180.0f; // 기본 90도 -> 180도로 확장 (매우 넓은 시야각)
	SightRange = 2500.0f; // 기본 800 -> 2500으로 대폭 확장 (매우 넓은 시야)

	// 스캐빈저 특화 소리 감지 (더 예민함)
	SoundDetectionRange = 2000.0f; // 기본 1500 -> 2000으로 확장
	NoiseDetectionRange = 1500.0f; // 기본 1000 -> 1500으로 확장
}

void ATerminidScavenger::BeginPlay()
{
	// 먼저 스캐빈저 스탯으로 초기화
	FTerminidStats ScavengerStats = FTerminidStats::CreateScavengerStats();
	InitializeTerminid(ScavengerStats, ETerminidType::Scavenger);

	// Flee 상태 초기화
	bIsFleeingActive = false;
	FleeTargetLocation = FVector::ZeroVector;
	FleeStartTime = 0.0f;

	// Idle 상태 초기화
	bHasIdleTarget = false;
	IdleTargetLocation = FVector::ZeroVector;

	// Chase 상태 초기화
	bIsZigzagging = false;
	ZigzagTargetLocation = FVector::ZeroVector;
	LastZigzagTime = 0.0f;

	// 그 다음 부모 BeginPlay 호출 (여기서 burrow 체크가 올바르게 동작)
	Super::BeginPlay();
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
	// 먼저 부모 클래스의 기본 Idle 동작 실행
	Super::ProcessIdleBehavior(DeltaTime);

	// 스캐빈저 특화 대기 행동
	// 애니메이션 이벤트 호출
	OnIdleAnimation();

	// 랜덤 이동 로직
	ProcessIdleMovement(DeltaTime);
}

void ATerminidScavenger::ProcessIdleMovement(float DeltaTime)
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	FVector CurrentLocation = GetActorLocation();

	// 목표 지점이 없거나 도달했으면 새로운 목표 설정
	if (!bHasIdleTarget || FVector::Dist(CurrentLocation, IdleTargetLocation) < IdleTargetReachThreshold)
	{
		// 목표 도달 시 잠시 멈춤 (자연스러운 대기)
		if (bHasIdleTarget)
		{
			bHasIdleTarget = false;
			LastGroupCheckTime = CurrentTime; // 대기 시간 시작
			return; // 이번 프레임은 멈춤
		}

		// 5초마다만 새로운 목표 설정 (더 긴 간격으로 변경)
		if (CurrentTime - LastGroupCheckTime > 5.0f)
		{
			LastGroupCheckTime = CurrentTime;

			// 더 작고 자연스러운 이동 목표 설정
			FVector RandomOffset = FVector(
				FMath::RandRange(-120.0f, 120.0f), // 범위 줄이기
				FMath::RandRange(-120.0f, 120.0f),
				0.0f
			);

			IdleTargetLocation = CurrentLocation + RandomOffset;
			bHasIdleTarget = true;

			// UE_LOG(LogTemp, Warning, TEXT("Scavenger New Idle Target: %s"), *IdleTargetLocation.ToString());
		}
	}

	// 목표 지점이 있으면 천천히 이동 (속도 조절)
	if (bHasIdleTarget)
	{
		// 느린 속도로 이동 (DeltaTime에 0.3 배수 적용)
		FVector Direction = (IdleTargetLocation - CurrentLocation).GetSafeNormal2D();

		// CharacterMovementComponent에 느린 입력 적용
		UCharacterMovementComponent* CharMovement = GetCharacterMovement();
		if (CharMovement)
		{
			CharMovement->AddInputVector(Direction * 0.3f); // 30% 속도로 천천히 이동
		}
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

	// 지그재그 추적 처리
	ProcessZigzagChase(DeltaTime);
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
	// 개선된 도주 로직 - 500유닛까지만 도주
	if (HasValidTarget())
	{
		// 도주 시작 위치와 목표 설정
		if (!bIsFleeingActive)
		{
			bIsFleeingActive = true;
			FleeStartTime = GetWorld()->GetTimeSeconds();

			// 플레이어 반대 방향으로 500유닛 떨어진 위치 계산
			FVector FleeDirection = (GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal();
			FleeTargetLocation = GetActorLocation() + (FleeDirection * FleeDistance);
		}

		// 목표 지점에 도달했는지 확인
		float DistanceToFleeTarget = FVector::Dist(GetActorLocation(), FleeTargetLocation);
		if (DistanceToFleeTarget > 50.0f) // 50유닛 이내면 도착으로 간주
		{
			// 목표 지점을 향해 이동
			MoveTowardsLocation(FleeTargetLocation, DeltaTime);
		}
		else
		{
			// 도주 완료 - 제자리에서 대기하며 체력 회복 기다림
			StopMovement();
		}

		// 10초 이상 도주했으면 강제로 idle로 전환 (무한 도주 방지)
		float CurrentTime = GetWorld()->GetTimeSeconds();
		if (CurrentTime - FleeStartTime > FLEE_TIMEOUT)
		{
			bIsFleeingActive = false;
			if (StateMachine)
			{
				StateMachine->ChangeState(ETerminidState::Idle);
			}
		}
	}
	else
	{
		// 타겟이 없으면 도주 상태 리셋
		bIsFleeingActive = false;
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
	{
		return false;
	}

	// 도주 상태에서는 공격하지 않음
	if (ShouldFleeFromCombat())
	{
		return false;
	}

	return true;
}

void ATerminidScavenger::PerformAttack()
{
	if (!CanPerformAttack())
	{
		return;
	}

	// 기본 공격 수행
	Super::PerformAttack();
}

void ATerminidScavenger::Die()
{
	// 죽음 애니메이션 호출
	OnDeathAnimation();

	// Flee 상태 리셋
	bIsFleeingActive = false;

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

void ATerminidScavenger::ProcessZigzagChase(float DeltaTime)
{
	if (!HasValidTarget())
	{
		return;
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();
	FVector CurrentLocation = GetActorLocation();

	// 지그재그 중이 아닐 때 새로운 지그재그 시작
	if (!bIsZigzagging && CurrentTime - LastZigzagTime > 1.2f) // 1.2초마다
	{
		LastZigzagTime = CurrentTime;
		bIsZigzagging = true;

		// 좌우로 큰 사이드스텝
		FVector SideStep = GetActorRightVector() * FMath::RandRange(-200.0f, 200.0f);
		ZigzagTargetLocation = CurrentLocation + SideStep;

		// UE_LOG(LogTemp, Warning, TEXT("Scavenger Zigzag Start: %s"), *ZigzagTargetLocation.ToString());
	}

	// 지그재그 중일 때
	if (bIsZigzagging)
	{
		float DistanceToZigzag = FVector::Dist(CurrentLocation, ZigzagTargetLocation);
		if (DistanceToZigzag > 70.0f) // 아직 지그재그 목표에 도달하지 못함
		{
			// 지그재그 목표로 이동
			MoveTowardsLocation(ZigzagTargetLocation, DeltaTime);
		}
		else
		{
			// 지그재그 완료, 이제 타겟 추적 재개
			bIsZigzagging = false;
		}
	}

	// 지그재그 중이 아니면 일반 추적
	if (!bIsZigzagging)
	{
		MoveTowardsLocation(CurrentTarget->GetActorLocation(), DeltaTime);
	}
}

// 스캐빈저 특화 사격 소리 감지 (더 예민한 반응)
void ATerminidScavenger::OnGunfireDetected(FVector FireLocation, float Range)
{
	// 부모 클래스 구현 먼저 호출
	Super::OnGunfireDetected(FireLocation, Range);

	// 스캐빈저는 사격 소리에 더 빠르게 반응
	// 체력이 낮으면 즉시 도주
	if (ShouldFleeFromCombat())
	{
		if (StateMachine)
		{
			StateMachine->ChangeState(ETerminidState::Flee);
		}
	}

	// 도움 요청
	if (bCanCallForHelp && !bHasCalledForHelp)
	{
		CallForHelp();
	}
}
