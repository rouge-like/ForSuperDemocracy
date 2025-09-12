// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponData.h"
#include "WeaponBase.generated.h"


class USkeletalMeshComponent;
class UWeaponComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFired, AWeaponBase*, Weapon);

UCLASS()
class FORSUPERDEMOCRACY_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
    // Sets default values for this actor's properties
    AWeaponBase();

protected:
    // BeginPlay: 스폰/시작 시 초기화 지점
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
    TObjectPtr<USkeletalMeshComponent> Mesh; // 무기 스켈레탈 메시(머즐 소켓 포함)

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
    TObjectPtr<UWeaponData> Data; // 무기 스펙/반동/스프레드/ADS 데이터

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
    FName MuzzleSocketName = TEXT("attach_muzzle"); // 머즐 소켓명(총구 기준)

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
    float MaxRange = 10000.f;     // 히트스캔 최대 사거리(cm)

    int32 CurrentAmmo = 0;        // 현재 탄약(탄창 내)
    bool bIsReloading = false;    // 재장전 중 여부
    bool bIsFiring = false;       // 연사 중 여부
    UPROPERTY(BlueprintReadOnly)
    bool bIsAiming = false;       // ADS 상태 여부(발사 게이팅)
    float FiringTime = 0.f;       // 마지막 발사 이후 경과시간(연사 타이밍)

    FTimerHandle TimerHandle_Reload;

    virtual void FireOnce();              // 1발 발사 처리
    bool CanFire() const;         // 발사 가능 조건 체크(재장전/탄 수 등)

    FVector GetMuzzleLocation() const; // 머즐 위치 조회
    FRotator GetFireRotation() const;  // 발사 기준 회전 조회(오너 뷰/머즐)

    void EndReload();             // 재장전 완료(타이머 콜백)
    
    void ShowBullet() const;      // 디버그: 잔탄 표기

    UPROPERTY()
    TObjectPtr<UWeaponComponent> WC; // 소유자 무기 컴포넌트(탄약 풀 연동)
    UPROPERTY()
    TObjectPtr<APlayerController> PC;

    UPROPERTY(EditDefaultsOnly)
    FAimViewParams AimViewParams; // 무기 자체에 바인딩된 ADS 파라미터(옵션)

    // Recoil/Spread state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon|Spread")
    float CurrentBloom = 0.f;     // 누적 블룸(연사 시 증가, 시간 경과로 회복)

    // Computes current spread in degrees considering ADS and bloom
    float GetCurrentSpreadDegrees() const; // 현재 스프레드(기본 + 블룸, 도 단위)

    void ApplyRecoilKick();       // 반동: 컨트롤러에 피치/요 입력 적용

    float RecoilPitchToRecover = 0.f;
    float RecoilYawToRecover = 0.f;
public:	
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    void RegisterWeaponComponent(UWeaponComponent* wc);
    virtual void StartFire();
    virtual void StopFire();
    virtual void StartReload();

    UFUNCTION(BlueprintCallable, Category="Weapon")
    int32 GetCurrentAmmo() const { return CurrentAmmo; }
    
    const FAimViewParams& GetAimViewParams() const { return AimViewParams; }
    void SetAiming(bool aiming) { bIsAiming = aiming; }

    // 델리게이트
    FOnFired OnFired;
};
