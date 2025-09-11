#include "Huxley/TerminidBase.h"
#include "Huxley/TerminidFSM.h"
#include "Huxley/TerminidSpawner.h"
#include "Huxley/TerminidAIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "OSC/HealthComponent.h"

ATerminidBase::ATerminidBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Character 기본 설정 (CapsuleComponent와 MeshComponent는 이미 Character에 있음)
    GetCapsuleComponent()->SetCapsuleHalfHeight(88.0f);
    GetCapsuleComponent()->SetCapsuleRadius(34.0f);
    GetCapsuleComponent()->SetCollisionProfileName("Pawn");

    // 메시 설정
    GetMesh()->SetCollisionProfileName("NoCollision");

    // CharacterMovementComponent 설정
    UCharacterMovementComponent* CharMovement = GetCharacterMovement();
    if (CharMovement)
    {
        CharMovement->MaxWalkSpeed = 300.0f;
        CharMovement->MaxAcceleration = 1000.0f;
        CharMovement->BrakingDecelerationWalking = 2000.0f;
        CharMovement->bOrientRotationToMovement = true;
        CharMovement->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
        CharMovement->GravityScale = 1.0f;
        CharMovement->NavAgentProps.bCanCrouch = false;
        CharMovement->NavAgentProps.bCanJump = false;
        CharMovement->NavAgentProps.bCanWalk = true;
        CharMovement->NavAgentProps.bCanSwim = false;
        CharMovement->NavAgentProps.bCanFly = false;
    }

    // FSM 컴포넌트 설정
    StateMachine = CreateDefaultSubobject<UTerminidFSM>(TEXT("StateMachine"));

    // HealthComponent 생성
    Health = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

    // AI Controller 설정
    AIControllerClass = ATerminidAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    // 기본값 초기화
    TerminidType = ETerminidType::Scavenger;
    ParentSpawner = nullptr;
    CurrentTarget = nullptr;
    LastKnownTargetLocation = FVector::ZeroVector;
    bHasTarget = false;
    DistanceToTarget = 0.0f;
    LastAttackTime = 0.0f;
    bCanAttack = true;
    
    // 애니메이션 관련 초기화
    bIsSpawning = false;
    bIsPlayingAnimation = false;
    SpawnAnimationDuration = 2.0f;
}

void ATerminidBase::BeginPlay()
{
    Super::BeginPlay();
    
    // HealthComponent 초기화 및 이벤트 바인딩
    if (Health)
    {
        Health->OnDamaged.AddDynamic(this, &ATerminidBase::OnDamaged);
        Health->OnDeath.AddDynamic(this, &ATerminidBase::OnHealthComponentDeath);
    }
    
    // 이동 속도를 스탯에 맞게 설정
    UCharacterMovementComponent* CharMovement = GetCharacterMovement();
    if (CharMovement)
    {
        CharMovement->MaxWalkSpeed = BaseStats.MoveSpeed;
    }
    
    // 플레이어 감지 초기화
    LastPlayerDetectionTime = 0.0f;
}

void ATerminidBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // 스폰 중일 때는 AI 행동 비활성화
    if (bIsSpawning)
    {
        return;
    }
    
    // 플레이어 감지 (최적화를 위해 일정 간격마다만 실행)
    UpdatePlayerDetection(DeltaTime);
    
    // FSM 업데이트
    if (StateMachine)
    {
        StateMachine->UpdateStateMachine(DeltaTime);
    }
    
    // 타겟과의 거리 업데이트
    if (bHasTarget && CurrentTarget)
    {
        UpdateTargetDistance();
    }
    
    // 이동 업데이트
    UpdateMovement(DeltaTime);
}

void ATerminidBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
}

void ATerminidBase::InitializeTerminid(const FTerminidStats& Stats, ETerminidType Type)
{
    BaseStats = Stats;
    TerminidType = Type;
    
    // HealthComponent 초기화
    if (Health)
    {
        // HealthComponent의 MaxHealth 설정이 필요하다면 여기에 추가
        // Health->MaxHealth = BaseStats.Health; // HealthComponent에서 해당 속성이 Protected이면 수정 불가
    }
    
    // 이동 속도 적용
    UCharacterMovementComponent* CharMovement = GetCharacterMovement();
    if (CharMovement)
    {
        CharMovement->MaxWalkSpeed = BaseStats.MoveSpeed;
    }
}

void ATerminidBase::SetParentSpawner(ATerminidSpawner* Spawner)
{
    ParentSpawner = Spawner;
}

// 성능 최적화를 위한 C++ 구현
void ATerminidBase::ProcessIdleBehavior(float DeltaTime)
{
    // 기본 대기 상태 처리
    // 파생 클래스에서 오버라이드하여 구체적인 행동 구현
}

void ATerminidBase::ProcessPatrolBehavior(float DeltaTime)
{
    // 기본 순찰 상태 처리
    // 파생 클래스에서 오버라이드하여 구체적인 행동 구현
}

void ATerminidBase::ProcessChaseBehavior(float DeltaTime)
{
    // 기본 추적 행동 - 타겟을 향해 이동
    if (HasValidTarget())
    {
        MoveTowardsTarget(DeltaTime);
    }
}

void ATerminidBase::ProcessAttackBehavior(float DeltaTime)
{
    // 기본 공격 행동
    if (CanPerformAttack())
    {
        PerformAttack();
    }
}

void ATerminidBase::ProcessHurtBehavior(float DeltaTime)
{
    // 기본 피격 상태 처리 - 잠시 멈춤
    StopMovement();
}

void ATerminidBase::ProcessSwarmBehavior(float DeltaTime)
{
    // 기본 군집 행동 - 추적과 유사하지만 협력적
    ProcessChaseBehavior(DeltaTime);
}

void ATerminidBase::ProcessFleeBehavior(float DeltaTime)
{
    // 기본 도주 행동 - 타겟에서 멀어지기
    if (HasValidTarget())
    {
        FVector FleeDirection = GetActorLocation() - CurrentTarget->GetActorLocation();
        FleeDirection.Normalize();
        
        FVector FleeLocation = GetActorLocation() + (FleeDirection * 1000.0f);
        MoveTowardsLocation(FleeLocation, DeltaTime);
    }
}

void ATerminidBase::ProcessDeathBehavior(float DeltaTime)
{
    // 기본 죽음 상태 처리 - 이동 정지
    StopMovement();
}

// 타겟 관리
void ATerminidBase::SetCurrentTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    bHasTarget = (NewTarget != nullptr);
    
    if (bHasTarget)
    {
        LastKnownTargetLocation = NewTarget->GetActorLocation();
        UpdateTargetDistance();
    }
}

void ATerminidBase::ClearTarget()
{
    CurrentTarget = nullptr;
    bHasTarget = false;
    DistanceToTarget = 0.0f;
}

bool ATerminidBase::HasValidTarget() const
{
    return bHasTarget && IsValid(CurrentTarget);
}

void ATerminidBase::UpdateTargetDistance()
{
    if (HasValidTarget())
    {
        DistanceToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
        LastKnownTargetLocation = CurrentTarget->GetActorLocation();
    }
}

bool ATerminidBase::IsTargetInRange(float Range) const
{
    return HasValidTarget() && (DistanceToTarget <= Range);
}

bool ATerminidBase::IsTargetInAttackRange() const
{
    return IsTargetInRange(BaseStats.AttackRange);
}

// 전투
bool ATerminidBase::CanPerformAttack() const
{
    if (!IsAlive() || !bCanAttack || !IsTargetInAttackRange())
    {
        return false;
    }
    
    // 공격 쿨다운 체크
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastAttackTime) >= BaseStats.AttackCooldown;
}

void ATerminidBase::PerformAttack()
{
    if (!CanPerformAttack())
    {
        return;
    }
    
    LastAttackTime = GetWorld()->GetTimeSeconds();
    
    // 블루프린트 이벤트 호출 (애니메이션 처리용)
    ExecuteAttackBehavior();
    
    // 실제 데미지는 파생 클래스에서 구현
}

void ATerminidBase::OnDamaged(float Damage, AActor* DamageCauser, AController* EventInstigator,
    TSubclassOf<UDamageType> DamageType)
{
    // FSM 상태 변경 - Hurt 상태로
    if (StateMachine && IsAlive())
    {
        StateMachine->ChangeState(ETerminidState::Hurt);
    }

    // 데미지를 준 적을 타겟으로 설정
    if (DamageCauser && !CurrentTarget)
    {
        SetCurrentTarget(DamageCauser);
    }

    // 블루프린트 이벤트 호출
    OnDamageReceived(Damage, DamageCauser);
}

// HealthComponent OnDeath 이벤트 핸들러
void ATerminidBase::OnHealthComponentDeath(AActor* Victim)
{
    // FSM 상태를 Death로 변경
    if (StateMachine)
    {
        StateMachine->ChangeState(ETerminidState::Death);
    }
    
    // 이동 정지
    StopMovement();
    
    // 블루프린트 이벤트 호출
    OnDeath();
    
    // 스포너에게 죽음 알림
    HandleDeath();
    
    // 일정 시간 후 액터 제거
    FTimerHandle DeathTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(
        DeathTimerHandle,
        [this]() { Destroy(); },
        3.0f,
        false
    );
}

// HealthComponent 기반 유틸리티 함수들
float ATerminidBase::GetCurrentHealth() const
{
    return Health ? Health->GetHealthPercent() * GetMaxHealth() : 0.0f;
}

float ATerminidBase::GetMaxHealth() const
{
    return BaseStats.Health;
}

float ATerminidBase::GetHealthPercent() const
{
    return Health ? Health->GetHealthPercent() : 0.0f;
}

bool ATerminidBase::IsAlive() const
{
    return Health ? Health->IsAlive() : false;
}

// 이동 유틸리티
void ATerminidBase::MoveTowardsTarget(float DeltaTime)
{
    if (!HasValidTarget())
    {
        return;
    }
    
    MoveTowardsLocation(CurrentTarget->GetActorLocation(), DeltaTime);
}

void ATerminidBase::MoveTowardsLocation(const FVector& TargetLocation, float DeltaTime)
{
    UCharacterMovementComponent* CharMovement = GetCharacterMovement();
    if (!CharMovement)
    {
        return;
    }
    
    FVector Direction = (TargetLocation - GetActorLocation()).GetSafeNormal2D(); // 2D로 제한하여 Y축 회전만
    CharMovement->AddInputVector(Direction);
    
    // CharacterMovementComponent가 자동으로 회전을 처리하므로 bOrientRotationToMovement = true일 때 수동 회전 불필요
}

void ATerminidBase::StopMovement()
{
    UCharacterMovementComponent* CharMovement = GetCharacterMovement();
    if (CharMovement)
    {
        CharMovement->StopMovementImmediately();
    }
}

// 죽음 및 생명주기 - HealthComponent를 통해 처리
void ATerminidBase::Die()
{
    if (!IsAlive())
    {
        return;
    }
    
    // HealthComponent의 Kill 함수를 호출하여 죽음 처리
    if (Health)
    {
        Health->Kill();
    }
}

// 플레이어 감지 (기본 FSM 방식)
void ATerminidBase::UpdatePlayerDetection(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // 일정 간격마다만 감지 체크 (성능 최적화)
    if (CurrentTime - LastPlayerDetectionTime < PLAYER_DETECTION_INTERVAL)
    {
        return;
    }
    
    LastPlayerDetectionTime = CurrentTime;
    
    // 가장 가까운 플레이어 찾기
    APawn* NearestPlayer = FindNearestPlayer();
    
    if (NearestPlayer)
    {
        // 감지 범위 및 시야각 체크
        if (IsPlayerInDetectionRange(NearestPlayer) && IsPlayerInSight(NearestPlayer))
        {
            // 타겟이 없으면 새로운 타겟으로 설정
            if (!CurrentTarget)
            {
                SetCurrentTarget(NearestPlayer);
                
                // FSM 상태를 Chase로 변경
                if (StateMachine)
                {
                    StateMachine->ChangeState(ETerminidState::Chase);
                }
            }
        }
    }
}

APawn* ATerminidBase::FindNearestPlayer() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    APawn* NearestPlayer = nullptr;
    float NearestDistance = FLT_MAX;
    FVector MyLocation = GetActorLocation();
    
    // 모든 플레이어 컨트롤러를 찾아서 가장 가까운 플레이어 반환
    for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
    {
        APlayerController* PC = Iterator->Get();
        if (PC && PC->GetPawn())
        {
            float Distance = FVector::Dist(MyLocation, PC->GetPawn()->GetActorLocation());
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestPlayer = PC->GetPawn();
            }
        }
    }
    
    return NearestPlayer;
}

bool ATerminidBase::IsPlayerInSight(APawn* Player) const
{
    if (!Player)
    {
        return false;
    }
    
    FVector MyLocation = GetActorLocation();
    FVector PlayerLocation = Player->GetActorLocation();
    FVector MyForward = GetActorForwardVector();
    
    // 플레이어 방향 벡터
    FVector ToPlayer = (PlayerLocation - MyLocation).GetSafeNormal();
    
    // 내적을 이용한 시야각 계산 (90도 시야각)
    float DotProduct = FVector::DotProduct(MyForward, ToPlayer);
    float ViewAngleCos = FMath::Cos(FMath::DegreesToRadians(90.0f)); // 90도 = 180도 시야각
    
    return DotProduct > ViewAngleCos;
}

bool ATerminidBase::IsPlayerInDetectionRange(APawn* Player) const
{
    if (!Player)
    {
        return false;
    }
    
    float Distance = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
    return Distance <= BaseStats.GetActualDetectionRange();
}

// Private 함수들

void ATerminidBase::UpdateMovement(float DeltaTime)
{
    // 기본 이동 업데이트 로직
    // 필요에 따라 확장 가능
}

void ATerminidBase::HandleDeath()
{
    if (ParentSpawner)
    {
        // 스포너에게 죽음 알림 (스포너에서 구현된 함수 호출)
        // ParentSpawner->OnMonsterDeath(this);
    }
}

// 스폰 시퀀스 관련 함수들
void ATerminidBase::StartSpawnSequence()
{
    bIsSpawning = true;
    bIsPlayingAnimation = true;
    
    // FSM을 스폰 준비 상태로 설정 (움직임 비활성화)
    if (StateMachine)
    {
        StateMachine->ChangeState(ETerminidState::Idle);
    }
    
    // 이동 비활성화
    StopMovement();
    
    // Blueprint 애니메이션 이벤트 호출
    OnSpawnAnimationStart();
    
    // 스폰 애니메이션 완료 타이머 설정
    GetWorld()->GetTimerManager().SetTimer(
        SpawnTimerHandle,
        [this]() { CompleteSpawnSequence(); },
        SpawnAnimationDuration,
        false
    );
}

void ATerminidBase::CompleteSpawnSequence()
{
    bIsSpawning = false;
    bIsPlayingAnimation = false;
    
    // Blueprint 애니메이션 완료 이벤트 호출
    OnSpawnAnimationComplete();
    
    // AI 활성화 - Idle 상태로 전환하여 정상적인 AI 시작
    if (StateMachine)
    {
        StateMachine->ChangeState(ETerminidState::Idle);
    }
    
    // 타이머 핸들 정리
    GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
}

#if WITH_EDITOR
void ATerminidBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    
    // 에디터에서 속성 변경 시 처리
    if (PropertyChangedEvent.Property)
    {
        FName PropertyName = PropertyChangedEvent.Property->GetFName();
        
        if (PropertyName == GET_MEMBER_NAME_CHECKED(ATerminidBase, BaseStats))
        {
            // HealthComponent 최대체력은 HealthComponent 내부에서 관리
            UCharacterMovementComponent* CharMovement = GetCharacterMovement();
            if (CharMovement)
            {
                CharMovement->MaxWalkSpeed = BaseStats.MoveSpeed;
            }
        }
    }
}
#endif