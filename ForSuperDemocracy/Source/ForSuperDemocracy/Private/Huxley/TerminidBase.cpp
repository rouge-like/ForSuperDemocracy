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
    LastHurtTime = 0.0f;
    HurtRecoveryTime = 1.5f; // 1.5초 후 복원
    
    // Burrow 관련 초기화
    bStartInBurrow = false;
    bIsBurrowed = false;
    BurrowDetectionRange = 800.0f;
    BurrowEmergeDuration = 2.0f;
    
    // 애니메이션 관련 초기화
    bIsSpawning = false;
    bIsPlayingAnimation = false;
    bIsPlayingAttackAnimation = false;
    SpawnAnimationDuration = 2.0f;
    AttackAnimationDuration = 1.0f; // 공격 애니메이션 기본 1초
    
    // 체력 회복 관련 초기화
    HealthRegenerationRate = 10.0f; // 초당 10 체력 회복
    HealthRegenerationDelay = 0.0f; // 지연 시간 없이 상시 회복
    LastDamageTime = 0.0f;
    bIsRegeneratingHealth = false;

    // 소음 감지 관련 초기화
    NoiseDetectionRange = 1000.0f; // 기본 1000 유닛
    LastHeardNoiseLocation = FVector::ZeroVector;
    LastNoiseTime = 0.0f;
    NoiseResponseDuration = 10.0f; // 10초간 각성 상태
}

void ATerminidBase::BeginPlay()
{
    Super::BeginPlay();
    
    // HealthComponent 초기화 및 이벤트 바인딩
    if (Health)
    {
        Health->OnDamaged.AddDynamic(this, +&ATerminidBase::OnDamaged);
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
    
    // 소음 감지 시스템 등록
    RegisterForNoiseEvents();
    
    // Burrow 상태 초기화
    if (bStartInBurrow && TerminidType == ETerminidType::Scavenger)
    {
        StartBurrowState();
    }
}

void ATerminidBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 타이머 정리
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
        GetWorld()->GetTimerManager().ClearTimer(HurtRecoveryTimerHandle);
        GetWorld()->GetTimerManager().ClearTimer(BurrowEmergeTimerHandle);
        GetWorld()->GetTimerManager().ClearTimer(HealthRegenerationTimerHandle);
    }
    
    Super::EndPlay(EndPlayReason);
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
    
    // 체력 회복 처리
    ProcessHealthRegeneration(DeltaTime);

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
    // 기본 추적 행동 - 타겟을 향해 이동하다가 공격 범위에 들어오면 공격
    if (HasValidTarget())
    {
        // 공격 범위에 도달하면 Attack 상태로 전환
        if (IsTargetInAttackRange() && CanPerformAttack())
        {
            if (StateMachine)
            {
                StateMachine->ChangeState(ETerminidState::Attack);
            }
            return;
        }
        
        // 아직 공격 범위가 아니면 계속 이동
        MoveTowardsTarget(DeltaTime);
    }
}

void ATerminidBase::ProcessAttackBehavior(float DeltaTime)
{
    // 공격 애니메이션 재생 중이면 상태 변경 금지 및 이동 중단
    if (bIsPlayingAttackAnimation)
    {
        // 공격 중에는 움직임 중단 (자연스러운 공격을 위해)
        StopMovement();
        // 공격 애니메이션이 완료될 때까지 대기
        return;
    }
    
    // 기본 공격 행동
    if (HasValidTarget())
    {
        // 타겟이 공격 범위를 벗어났으면 다시 추적 (애니메이션 끝난 후에만)
        if (!IsTargetInAttackRange())
        {
            if (StateMachine)
            {
                StateMachine->ChangeState(ETerminidState::Chase);
            }
            return;
        }
        
        // 공격 범위 내에 있으면 공격 수행
        if (CanPerformAttack())
        {
            PerformAttack();
        }
    }
    else
    {
        // 타겟이 없으면 Idle로 전환
        if (StateMachine)
        {
            StateMachine->ChangeState(ETerminidState::Idle);
        }
    }
}

void ATerminidBase::ProcessHurtBehavior(float DeltaTime)
{
    // 피격 상태에서는 이동만 중단, 멈추지 않음 (ABP에서 애니메이션 처리)
    // 복원은 타이머를 통해 자동으로 처리됨
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

void ATerminidBase::ProcessBurrowBehavior(float DeltaTime)
{
    // Burrow 상태에서는 감지 체크만 수행
    if (bIsBurrowed)
    {
        CheckBurrowDetection(DeltaTime);
    }
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
    
    // 공격 애니메이션 시작
    StartAttackAnimation();
    
    // 블루프린트 이벤트 호출 (애니메이션 처리용)
    ExecuteAttackBehavior();
    
    // 실제 데미지는 파생 클래스에서 구현
}

void ATerminidBase::OnDamaged(float Damage, AActor* DamageCauser, AController* EventInstigator,
    TSubclassOf<UDamageType> DamageType)
{
    // 피격 시간 기록
    LastHurtTime = GetWorld()->GetTimeSeconds();

    // 데미지 시간 업데이트 (체력 회복용)
    LastDamageTime = GetWorld()->GetTimeSeconds();

    // 체력 회복 중단
    StopHealthRegeneration();

    // 공격 애니메이션 중단 (피격 시에는 애니메이션 중단 허용)
    if (bIsPlayingAttackAnimation)
    {
        CompleteAttackAnimation();
    }

    // 기존 타이머 취소
    GetWorld()->GetTimerManager().ClearTimer(HurtRecoveryTimerHandle);

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

    // 복원 타이머 시작
    GetWorld()->GetTimerManager().SetTimer(
        HurtRecoveryTimerHandle,
        this,
        &ATerminidBase::RecoverFromHurt,
        HurtRecoveryTime,
        false
    );

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
    
    // 레그돌 활성화 및 충돌 처리
    EnableRagdoll();
    DisableCollisionWithPlayersAndTerminids();
    
    // 블루프린트 이벤트 호출
    OnDeath();
    
    // 스포너에게 죽음 알림
    HandleDeath();
    
    // 일정 시간 후 액터 제거 (레그돌 상태 유지를 위해 시간 증가)
    FTimerHandle DeathTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(
        DeathTimerHandle,
        [this]() { Destroy(); },
        10.0f, // 10초 후 제거
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

// 피격 상태에서 복원
void ATerminidBase::RecoverFromHurt()
{
    // 살아있고 현재 Hurt 상태일 때만 복원
    if (!IsAlive() || !StateMachine)
    {
        return;
    }
    
    // 먼저 상태 변경 락을 해제
    StateMachine->SetStateChangeLock(false);
    
    // 타겟이 있으면 Chase 상태로, 없으면 Idle 상태로 복원
    if (HasValidTarget())
    {
        StateMachine->ChangeState(ETerminidState::Chase);
    }
    else
    {
        StateMachine->ChangeState(ETerminidState::Idle);
    }
    
    // 타이머 핸들 정리
    GetWorld()->GetTimerManager().ClearTimer(HurtRecoveryTimerHandle);
}

// Burrow 관련 함수들
void ATerminidBase::StartBurrowState()
{
    bIsBurrowed = true;
    
    // FSM을 Burrow 상태로 변경
    if (StateMachine)
    {
        StateMachine->ChangeState(ETerminidState::Burrow);
    }
    
    // 충돌 비활성화 (플레이어와 다른 Terminid와 충돌 안함)
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    GetMesh()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    
    // 블루프린트 이벤트 호출 (숨기 애니메이션)
    ExecuteBurrowBehavior();
}

void ATerminidBase::EmergeFromBurrow()
{
    // 이머지 애니메이션 시작
    GetWorld()->GetTimerManager().SetTimer(
        BurrowEmergeTimerHandle,
        [this]() {
            // 이머지 완료 후 처리
            bIsBurrowed = false;
            
            // 충돌 활성화
            GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
            GetMesh()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
            
            // Idle 상태로 전환하여 정상 AI 시작
            if (StateMachine)
            {
                StateMachine->ChangeState(ETerminidState::Idle);
            }
            
            GetWorld()->GetTimerManager().ClearTimer(BurrowEmergeTimerHandle);
        },
        BurrowEmergeDuration,
        false
    );
}

void ATerminidBase::CheckBurrowDetection(float DeltaTime)
{
    if (!bIsBurrowed)
    {
        return;
    }
    
    // 플레이어 감지
    APawn* NearestPlayer = FindNearestPlayer();
    if (NearestPlayer)
    {
        float Distance = FVector::Dist(GetActorLocation(), NearestPlayer->GetActorLocation());
        if (Distance <= BurrowDetectionRange)
        {
            // 플레이어가 감지 범위에 들어오면 이머지
            EmergeFromBurrow();
        }
    }
}

// 레그돌 및 충돌 관리
void ATerminidBase::EnableRagdoll()
{
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (MeshComp)
    {
        // 레그돌 활성화
        MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
        MeshComp->SetSimulatePhysics(true);
        MeshComp->WakeAllRigidBodies();
        MeshComp->bBlendPhysics = true;
        
        // CharacterMovementComponent 비활성화
        UCharacterMovementComponent* CharMovement = GetCharacterMovement();
        if (CharMovement)
        {
            CharMovement->DisableMovement();
            CharMovement->StopMovementImmediately();
        }
    }
}

void ATerminidBase::DisableCollisionWithPlayersAndTerminids()
{
    // CapsuleComponent의 Pawn 충돌 비활성화 (플레이어와 다른 Terminid)
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    
    // 메시는 World Static과만 충돌 (바닥에 떨어뜨리기 위해)
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (MeshComp)
    {
        MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
        MeshComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
        MeshComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
    }
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
    // 공격 애니메이션 재생 중에는 이동하지 않음
    if (bIsPlayingAttackAnimation)
    {
        return;
    }
    
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
        bool bCanSeePlayer = IsPlayerInDetectionRange(NearestPlayer) && IsPlayerInSight(NearestPlayer);
        bool bRecentNoiseAlert = false;
        
        // 최근 소음이 있었는지 확인 (각성 상태)
        if (LastNoiseTime > 0.0f && (CurrentTime - LastNoiseTime) < NoiseResponseDuration)
        {
            bRecentNoiseAlert = true;
        }
        
        // 직접 감지하거나 소음으로 인한 각성 상태에서 플레이어 발견
        if (bCanSeePlayer || (bRecentNoiseAlert && IsPlayerInDetectionRange(NearestPlayer)))
        {
            // 타겟이 없으면 새로운 타겟으로 설정
            if (!CurrentTarget)
            {
                SetCurrentTarget(NearestPlayer);
                
                // 버로우 상태라면 즉시 등장
                if (bIsBurrowed)
                {
                    EmergeFromBurrow();
                }
                
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

// 소음 감지 시스템
void ATerminidBase::OnNoiseHeard(FVector NoiseLocation, float NoiseRadius, AActor* NoiseInstigator)
{
    // 죽었거나 버로우 상태면 반응하지 않음
    if (!IsAlive() || bIsBurrowed)
    {
        return;
    }
    
    // 소음 범위 내에 있는지 확인
    if (!IsNoiseInRange(NoiseLocation, NoiseRadius))
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // 소음 정보 저장
    LastHeardNoiseLocation = NoiseLocation;
    LastNoiseTime = CurrentTime;
    
    // 소음 발생 지점이 플레이어 근처라면 타겟으로 설정
    if (APawn* NearestPlayer = FindNearestPlayer())
    {
        float DistanceToPlayer = FVector::Dist(NoiseLocation, NearestPlayer->GetActorLocation());
        
        // 플레이어가 소음 근처에 있으면 타겟으로 설정
        if (DistanceToPlayer < 500.0f) // 500 유닛 내
        {
            SetCurrentTarget(NearestPlayer);
            
            // 버로우 상태라면 즉시 등장
            if (bIsBurrowed)
            {
                EmergeFromBurrow();
            }
            
            // FSM 상태를 Chase로 변경
            if (StateMachine)
            {
                StateMachine->ChangeState(ETerminidState::Chase);
            }
        }
    }
}

void ATerminidBase::RegisterForNoiseEvents()
{
    // Blueprint에서 구현할 수 있도록 Blueprint Implementable Event로 남겨둠
    // 또는 간단한 C++ 구현:
    // 실제 게임에서는 노이즈 시스템과 연결하여 구현
}

bool ATerminidBase::IsNoiseInRange(FVector NoiseLocation, float NoiseRadius) const
{
    float DistanceToNoise = FVector::Dist(GetActorLocation(), NoiseLocation);
    
    // 소음의 반경과 자신의 감지 범위를 모두 고려
    float EffectiveRange = FMath::Max(NoiseRadius, NoiseDetectionRange);
    
    return DistanceToNoise <= EffectiveRange;
}

// 공격 애니메이션 관리 함수들
void ATerminidBase::StartAttackAnimation()
{
    bIsPlayingAttackAnimation = true;
    
    // 애니메이션 지속 시간 후 자동 완료 처리
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            AttackAnimationTimerHandle,
            this,
            &ATerminidBase::CompleteAttackAnimation,
            AttackAnimationDuration,
            false
        );
    }
}

void ATerminidBase::CompleteAttackAnimation()
{
    bIsPlayingAttackAnimation = false;
    
    // 타이머 정리
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(AttackAnimationTimerHandle);
    }
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

// 체력 회복 시스템
void ATerminidBase::StartHealthRegeneration()
{
    if (!bIsRegeneratingHealth && HealthRegenerationRate > 0.0f)
    {
        bIsRegeneratingHealth = true;

        // 회복 타이머 시작 (0.1초마다 체력 회복)
        GetWorld()->GetTimerManager().SetTimer(
            HealthRegenerationTimerHandle,
            [this]() {
                if (Health && IsAlive())
                {
                    float RegenAmount = HealthRegenerationRate * 0.1f; // 0.1초마다 회복량
                    Health->Heal(RegenAmount);

                    // 최대 체력에 도달하면 회복 중단
                    if (Health->GetHealthPercent() >= 1.0f)
                    {
                        StopHealthRegeneration();
                    }
                }
            },
            0.1f, // 0.1초 간격
            true  // 반복
        );
    }
}

void ATerminidBase::StopHealthRegeneration()
{
    if (bIsRegeneratingHealth)
    {
        bIsRegeneratingHealth = false;
        GetWorld()->GetTimerManager().ClearTimer(HealthRegenerationTimerHandle);
    }
}

void ATerminidBase::ProcessHealthRegeneration(float DeltaTime)
{
    if (!IsAlive() || HealthRegenerationRate <= 0.0f)
    {
        return;
    }

    // 상시 회복 시스템 - 지연 시간 없이 즐시 시작
    if (!bIsRegeneratingHealth)
    {
        StartHealthRegeneration();
    }
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