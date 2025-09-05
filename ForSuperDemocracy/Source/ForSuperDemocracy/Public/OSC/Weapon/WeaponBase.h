// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponData.h"
#include "WeaponBase.generated.h"


class USkeletalMeshComponent;
class UWeaponComponent;

UCLASS()
class FORSUPERDEMOCRACY_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
    // Sets default values for this actor's properties
    AWeaponBase();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
    TObjectPtr<USkeletalMeshComponent> Mesh;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
    TObjectPtr<UWeaponData> Data;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
    FName MuzzleSocketName = TEXT("attach_muzzle");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
    float MaxRange = 10000.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
    bool bUseOwnerView = true;

    int32 CurrentAmmo = 0;
    bool bIsReloading = false;
    bool bIsFiring = false;
    UPROPERTY(BlueprintReadOnly)
    bool bIsAiming = false;
    float FiringTime = 0.f;

    FTimerHandle TimerHandle_Reload;

    void FireOnce();
    bool CanFire() const;

    FVector GetMuzzleLocation() const;
    FRotator GetFireRotation() const;

    void EndReload();
    
    void ShowBullet() const;

    UPROPERTY()
    TObjectPtr<UWeaponComponent> WC;

    UPROPERTY(EditDefaultsOnly)
    FAimViewParams AimViewParams;

    void UpdateAimAlignment();

    UPROPERTY()
    TObjectPtr<AActor> Target;

    UPROPERTY(BlueprintReadOnly)
    FVector TargetWS;
public:	
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    void RegisterWeaponComponent(UWeaponComponent* wc);
    void StartFire();
    void StopFire();
    void StartReload();

    UFUNCTION(BlueprintCallable, Category="Weapon")
    int32 GetCurrentAmmo() const { return CurrentAmmo; }
    
    const FAimViewParams& GetAimViewParams() const { return AimViewParams; }
    void SetAiming(bool aiming) { bIsAiming = aiming; }
};
