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

    // (특별한 행동 오버라이드 없음 - 기본 AI만 사용)
};