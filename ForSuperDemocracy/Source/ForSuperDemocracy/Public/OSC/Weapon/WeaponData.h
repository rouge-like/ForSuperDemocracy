// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AmmoType.h"
#include "WeaponData.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FAimViewParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ADS")
    float TargetFOV;              // 목표 시야각(FOV)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ADS")
    float TargetArmLength;        // 스프링암 길이(3인칭 카메라 거리)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ADS")
    FVector SocketOffset;         // 카메라 소켓 오프셋(좌/우/상하 미세 조정)
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ADS")
	FVector TargetOffset;         // 카메라 타겟 오프셋(좌/우/상하 미세 조정)
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ADS")
    float InterpSpeed;            // 보간 속도(값이 클수록 빠르게 전환)
};
UCLASS(BlueprintType)
class FORSUPERDEMOCRACY_API UWeaponData : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EAmmoType AmmoType;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxSize;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Damage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float FireTime;               // 발사 간격(초). RPM = 60 / fireTime
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float ReloadTime;             // 재장전에 걸리는 시간(초)

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FAimViewParams ViewParams;    // ADS 전환 시 카메라/암 파라미터

    // 반동 파라미터(도 단위, 1발당 적용 범위)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Recoil")
    float RecoilPitchMin = 0.2f;  // 피치 최소 반동

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Recoil")
    float RecoilPitchMax = 0.6f;  // 피치 최대 반동

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Recoil")
    float RecoilYawMin = 0.0f;    // 요 최소 반동

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Recoil")
    float RecoilYawMax = 0.4f;    // 요 최대 반동

    // 스프레드/블룸 파라미터(도 단위)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Spread")
    float BaseSpread = 0.3f;      // 기본 스프레드(원뿔 반각)

    // 발사 1회당 블룸 증가량(도)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Spread")
    float SpreadIncreasePerShot = 0.2f;

    // 블룸 상한(도)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Spread")
    float SpreadMax = 3.0f;

    // 초당 블룸 회복량(도)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Spread")
    float SpreadRecoveryPerSec = 3.0f;

	// 반동 회복 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Recover")
	float RecoverDegPerSec = 60.f;

	// 사격 중 반동 회복 비율
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Recover")
	float RecoverWhileFiring = 0.5f;
};
