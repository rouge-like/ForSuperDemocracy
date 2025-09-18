#include "Huxley/TerminidCharger.h"
#include "Huxley/TerminidFSM.h"
#include "Engine/World.h"

ATerminidCharger::ATerminidCharger()
{
    // 차저는 더 큰 충돌 크기 (탱커 역할)
    GetCapsuleComponent()->SetCapsuleHalfHeight(120.0f);
    GetCapsuleComponent()->SetCapsuleRadius(50.0f);
}

void ATerminidCharger::BeginPlay()
{
    Super::BeginPlay();

    // 차저 전용 스탯으로 초기화 (고체력 탱커)
    FTerminidStats ChargerStats = FTerminidStats::CreateChargerStats();
    InitializeTerminid(ChargerStats, ETerminidType::Charger);
}