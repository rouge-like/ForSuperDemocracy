#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Engine/DamageEvents.h"
#include "AnimNotify_ChargerAttack.generated.h"

/**
 * Terminid Charger 공격 데미지를 처리하는 AnimNotify
 * 강력한 돌진/충격 공격 특화
 */
UCLASS()
class FORSUPERDEMOCRACY_API UAnimNotify_ChargerAttack : public UAnimNotify
{
    GENERATED_BODY()

public:
    UAnimNotify_ChargerAttack();

    // AnimNotify의 핵심 함수 - 애니메이션에서 호출됨
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
    // Charger 공격 설정값들
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charger Attack Settings", meta = (ClampMin = "30.0", ClampMax = "150.0"))
    float DamageAmount = 60.0f; // Charger는 높은 데미지

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charger Attack Settings", meta = (ClampMin = "100.0", ClampMax = "600.0"))
    float AttackRange = 150.0f; // 긴 공격 범위 (돌진)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charger Attack Settings", meta = (ClampMin = "60.0", ClampMax = "180.0"))
    float AttackAngle = 120.0f; // 넓은 공격 각도 (범위 공격)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charger Attack Settings")
    TSubclassOf<UDamageType> DamageTypeClass;

    // 넉백 효과 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charger Attack Settings", meta = (ClampMin = "0.0", ClampMax = "2000.0"))
    float KnockbackForce = 800.0f; // 넉백 힘

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charger Attack Settings", meta = (ClampMin = "0.0", ClampMax = "500.0"))
    float KnockbackUpForce = 200.0f; // 위쪽 넉백 힘

    // 디버그 표시 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowDebug = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    float DebugDrawTime = 2.0f;

private:
    // 공격 범위 내 모든 타겟 찾기 (범위 공격)
    TArray<AActor*> FindTargetsInRange(const FVector& AttackerLocation, const FVector& AttackerForward, UWorld* World);

    // 타겟이 공격 범위와 각도 내에 있는지 확인
    bool IsTargetInAttackRange(const FVector& AttackerLocation, const FVector& AttackerForward, const FVector& TargetLocation);

    // HealthComponent를 통한 데미지 적용
    void ApplyDamageToTarget(AActor* Target, AActor* Attacker, float Damage);

    // 넉백 효과 적용
    void ApplyKnockbackToTarget(AActor* Target, const FVector& AttackerLocation);

    // 디버그 표시
    void DrawDebugAttackRange(UWorld* World, const FVector& AttackerLocation, const FVector& AttackerForward);
};