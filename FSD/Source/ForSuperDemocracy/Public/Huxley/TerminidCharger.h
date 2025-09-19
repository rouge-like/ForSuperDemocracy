#pragma once

#include "CoreMinimal.h"
#include "Huxley/TerminidBase.h"
#include "TerminidCharger.generated.h"

UCLASS()
class FORSUPERDEMOCRACY_API ATerminidCharger : public ATerminidBase
{
    GENERATED_BODY()

public:
    ATerminidCharger();

protected:
    virtual void BeginPlay() override;

public:
    // 차저 고유 속성들 - 탱커 특화
    // (별도 속성 없음 - 기본 스탯으로만 처리)

protected:
    // 공격 행동 오버라이드 - 차저 특화 공격 애니메이션
    virtual void ProcessAttackBehavior(float DeltaTime) override;

public:
    // Charger 특화 피격 시스템 - 50% 체력에서만 한 번 Hurt 상태
    void OnDamaged(float Damage, AActor* DamageCauser, AController* EventInstigator, TSubclassOf<UDamageType> DamageType);

private:
    // Charger 특화 피격 시스템
    bool bHasTriggeredHurtAt50Percent = false; // 50% 체력에서 Hurt 상태 발동했는지 여부
};