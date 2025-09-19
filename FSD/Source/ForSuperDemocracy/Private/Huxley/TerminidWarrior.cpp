#include "Huxley/TerminidWarrior.h"
#include "Huxley/TerminidFSM.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"

ATerminidWarrior::ATerminidWarrior()
{
    // 워리어 기본 설정 - 회피 비활성화
    DodgeChance = 0.0f; // 회피 확률 비활성화 (모든 총알 맞음)
    DodgeAnimationDuration = 1.0f; // 1초 회피 애니메이션

    // 내부 상태 초기화
    bIsDodging = false;
}

void ATerminidWarrior::BeginPlay()
{
    Super::BeginPlay();

    // 워리어 전용 스탯으로 초기화
    FTerminidStats WarriorStats = FTerminidStats::CreateWarriorStats();
    InitializeTerminid(WarriorStats, ETerminidType::Warrior);
}

// 상태 확인 함수들
bool ATerminidWarrior::IsDodging() const
{
    return bIsDodging;
}

void ATerminidWarrior::TriggerDodgeAnimation()
{
    if (bIsDodging)
        return;

    bIsDodging = true;

    // Blueprint 이벤트 호출 (회피 애니메이션 재생)
    OnDodgeStart();

    // 회피 애니메이션 지속 시간 후 종료
    GetWorld()->GetTimerManager().SetTimer(
        DodgeTimerHandle,
        [this]() {
            bIsDodging = false;
            OnDodgeEnd(); // Blueprint 이벤트 호출
        },
        DodgeAnimationDuration,
        false
    );
}

void ATerminidWarrior::ProcessAttackBehavior(float DeltaTime)
{
    // 회피 중에는 공격하지 않음
    if (bIsDodging)
    {
        return;
    }

    // 공격 애니메이션 호출
    OnAttackAnimation();

    // 기본 공격 행동
    Super::ProcessAttackBehavior(DeltaTime);
}

// 데미지 처리 오버라이드 - 회피 시스템
float ATerminidWarrior::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser)
{
    // 회피 중이면 데미지 무시
    if (bIsDodging)
        return 0.0f;

    // 40% 확률로 회피 시도
    if (FMath::RandRange(0.0f, 1.0f) <= DodgeChance)
    {
        TriggerDodgeAnimation();
        return 0.0f; // 회피 성공 시 데미지 무시
    }

    // 회피 실패 시 일반 데미지 처리
    return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}