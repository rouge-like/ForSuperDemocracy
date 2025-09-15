#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Engine/DamageEvents.h"
#include "AnimNotify_ScavengerAttack.generated.h"

/**
 * Scavenger 공격 데미지를 처리하는 AnimNotify
 * 공격 애니메이션의 특정 시점에서 플레이어에게 데미지를 입힘
 */
UCLASS()
class FORSUPERDEMOCRACY_API UAnimNotify_ScavengerAttack : public UAnimNotify
{
    GENERATED_BODY()

public:
    UAnimNotify_ScavengerAttack();

    // AnimNotify의 핵심 함수 - 애니메이션에서 호출됨
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
    // 데미지 설정값들
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Settings")
    float DamageAmount = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Settings")
    float AttackRange = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Settings")
    float AttackAngle = 90.0f; // 전방 90도 각도 내 공격

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Settings")
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