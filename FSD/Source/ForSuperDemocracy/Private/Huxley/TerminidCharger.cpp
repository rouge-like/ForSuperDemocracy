#include "Huxley/TerminidCharger.h"
#include "Huxley/TerminidFSM.h"
#include "OSC/HealthComponent.h"
#include "Engine/World.h"

ATerminidCharger::ATerminidCharger()
{
    // 차저는 더 큰 충돌 크기 (탱커 역할)
    GetCapsuleComponent()->SetCapsuleHalfHeight(120.0f);
    GetCapsuleComponent()->SetCapsuleRadius(50.0f);

    // Charger 특화 초기화
    bHasTriggeredHurtAt50Percent = false;

    // 차저 특화 감지 시스템 (향상된 감지 범위)
    SightAngle = 270.0f; // 기본 90도 -> 270도로 확장 (3배)
    SightRange = 2400.0f; // 기본 800 -> 2400으로 확장 (3배)
}

void ATerminidCharger::BeginPlay()
{
    // 차저 전용 스탯으로 초기화 (고체력 탱커)
    FTerminidStats ChargerStats = FTerminidStats::CreateChargerStats();
    InitializeTerminid(ChargerStats, ETerminidType::Charger);

    // 부모 BeginPlay 호출
    Super::BeginPlay();

    // Charger 전용 OnDamaged 델리게이트 바인딩을 다음 틱에 처리 (확실한 바인딩을 위해)
    GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
    {
        if (Health)
        {
            // 기존 바인딩 모두 제거
            Health->OnDamaged.RemoveAll(this);
            // Charger 전용 OnDamaged 바인딩
            Health->OnDamaged.AddDynamic(this, &ATerminidCharger::OnDamaged);
        }
    });
}

void ATerminidCharger::ProcessAttackBehavior(float DeltaTime)
{
    // 차저 특화 공격 애니메이션 호출
    OnAttackAnimation();

    // 기본 공격 행동
    Super::ProcessAttackBehavior(DeltaTime);
}

void ATerminidCharger::OnDamaged(float Damage, AActor* DamageCauser, AController* EventInstigator, TSubclassOf<UDamageType> DamageType)
{

    // 기본 피격 처리 (시간 기록, 체력 회복 중단 등)
    LastHurtTime = GetWorld()->GetTimeSeconds();
    LastDamageTime = GetWorld()->GetTimeSeconds();
    StopHealthRegeneration();

    // 데미지를 준 적을 타겟으로 설정
    if (DamageCauser && !CurrentTarget)
    {
        SetCurrentTarget(DamageCauser);
    }

    // Charger 특화: 50% 체력에서만 한 번 Hurt 상태 발동
    float HealthPercent = GetHealthPercent();

    if (!bHasTriggeredHurtAt50Percent && HealthPercent <= 0.5f && IsAlive())
    {
        // 50% 체력 도달 시 단 한 번만 Hurt 상태 발동
        bHasTriggeredHurtAt50Percent = true;

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

        // 복원 타이머 시작
        GetWorld()->GetTimerManager().SetTimer(
            HurtRecoveryTimerHandle,
            this,
            &ATerminidBase::RecoverFromHurt,
            HurtRecoveryTime,
            false
        );

    }
    else
    {
        // 50% 이상이거나 이미 Hurt를 발동했으면 Hurt 상태로 가지 않음
    }

    // 블루프린트 이벤트 호출
    OnDamageReceived(Damage, DamageCauser);
}