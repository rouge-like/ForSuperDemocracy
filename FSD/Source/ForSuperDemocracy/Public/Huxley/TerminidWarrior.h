#pragma once

#include "CoreMinimal.h"
#include "Huxley/TerminidBase.h"
#include "TerminidWarrior.generated.h"

UCLASS()
class FORSUPERDEMOCRACY_API ATerminidWarrior : public ATerminidBase
{
    GENERATED_BODY()

public:
    ATerminidWarrior();

protected:
    virtual void BeginPlay() override;

public:
    // 워리어 고유 속성들 - 회피 특화
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warrior", meta = (ClampMin = "0.1", ClampMax = "0.8"))
    float DodgeChance = 0.4f; // 피격 시 회피 확률 (40%)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warrior", meta = (ClampMin = "0.5", ClampMax = "3.0"))
    float DodgeAnimationDuration = 1.0f; // 회피 애니메이션 지속 시간

    // 워리어 상태 확인
    UFUNCTION(BlueprintPure, Category = "Warrior")
    bool IsDodging() const;

    // 워리어 전용 행동들
    UFUNCTION(BlueprintCallable, Category = "Warrior")
    void TriggerDodgeAnimation();

protected:
    // 데미지 처리 오버라이드 - 회피 시스템
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

    // 공격 행동 오버라이드 - 회피 중에는 공격하지 않음
    virtual void ProcessAttackBehavior(float DeltaTime) override;

private:
    // 회피 관련 변수들
    bool bIsDodging;
    FTimerHandle DodgeTimerHandle;

public:
    // Blueprint에서 사용할 이벤트들
    UFUNCTION(BlueprintImplementableEvent, Category = "Warrior")
    void OnDodgeStart();

    UFUNCTION(BlueprintImplementableEvent, Category = "Warrior")
    void OnDodgeEnd();
};