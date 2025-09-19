#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Engine/DamageEvents.h"
#include "AnimNotify_WarriorAttack.generated.h"

/**
 * Terminid Warrior 공격 데미지를 처리하는 AnimNotify
 * 빠르고 정확한 근접 공격 특화
 */
UCLASS()
class FORSUPERDEMOCRACY_API UAnimNotify_WarriorAttack : public UAnimNotify
{
    GENERATED_BODY()

public:
    UAnimNotify_WarriorAttack();

    // AnimNotify의 핵심 함수 - 애니메이션에서 호출됨
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
    // Warrior 공격 설정값들
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warrior Attack Settings", meta = (ClampMin = "10.0", ClampMax = "100.0"))
    float DamageAmount = 35.0f; // Warrior는 중간 데미지

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warrior Attack Settings", meta = (ClampMin = "50.0", ClampMax = "500.0"))
    float AttackRange = 100.0f; // 짧은 공격 범위

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warrior Attack Settings", meta = (ClampMin = "30.0", ClampMax = "180.0"))
    float AttackAngle = 60.0f; // 좁은 공격 각도 (정확한 공격)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warrior Attack Settings")
    TSubclassOf<UDamageType> DamageTypeClass;

    // 디버그 표시 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowDebug = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    float DebugDrawTime = 2.0f;

private:
    // 공격 범위 내 타겟 찾기
    AActor* FindTargetInRange(const FVector& AttackerLocation, const FVector& AttackerForward, UWorld* World);

    // 타겟이 공격 범위와 각도 내에 있는지 확인
    bool IsTargetInAttackRange(const FVector& AttackerLocation, const FVector& AttackerForward, const FVector& TargetLocation);

    // HealthComponent를 통한 데미지 적용
    void ApplyDamageToTarget(AActor* Target, AActor* Attacker, float Damage);

    // 디버그 표시
    void DrawDebugAttackRange(UWorld* World, const FVector& AttackerLocation, const FVector& AttackerForward);
};