// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OSC/Weapon/WeaponBase.h"
#include "Components/SkeletalMeshComponent.h" // for EAnimationMode
#include "Animation/AnimSequenceBase.h"
#include "HitscanWeapon.generated.h"

/**
 * 
 */
class UNiagaraSystem;
UCLASS()
class FORSUPERDEMOCRACY_API AHitscanWeapon : public AWeaponBase
{
    GENERATED_BODY()

protected:
    virtual void FireOnce() override;

    // 단발 발사 시 무기 메시에서 바로 재생할 애니메이션(ABP/몽타주 불필요)
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Animation")
    TObjectPtr<UAnimSequenceBase> FireAnim;

    // 발사 애니메이션 재생 전 상태를 복구하기 위한 캐시
    EAnimationMode::Type CachedAnimMode = EAnimationMode::AnimationBlueprint;
    TSubclassOf<UAnimInstance> CachedAnimClass = nullptr;
    FTimerHandle TimerHandle_RestoreAnim;

    UFUNCTION()
    void RestoreWeaponAnimMode();

    // 사격 이펙트
    UPROPERTY(EditDefaultsOnly, Category="Weapon|VFX")
    TObjectPtr<UNiagaraSystem> ShotVFX;

    // Optional rotation offset to align VFX forward with muzzle
    UPROPERTY(EditDefaultsOnly, Category="Weapon|VFX")
    FRotator ShotVFXRotationOffset = FRotator::ZeroRotator;
};

