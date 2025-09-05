// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "Components/ActorComponent.h"
#include "WeaponData.h"
#include "WeaponComponent.generated.h"

class AWeaponBase;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FORSUPERDEMOCRACY_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWeaponComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	int32 CurrentIdx;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ammo")
	TMap<EAmmoType, int32> AmmoPools;
public:	
    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(BlueprintReadOnly, Category="Weapon")
    TArray<TObjectPtr<AWeaponBase>> WeaponList;
	
	UFUNCTION()
	void Equip(int32 idx);

	UFUNCTION(BlueprintCallable)
	void StartFire();

	UFUNCTION(BlueprintCallable)
	void StopFire();

	UFUNCTION(BlueprintCallable)
	void Reload();

    UFUNCTION()
    int32 PullAmmo(EAmmoType type, int32 need);

    UFUNCTION(BlueprintCallable, Category="Weapon")
    void RegisterWeapon(AWeaponBase* Weapon);

    UFUNCTION(BlueprintCallable)
    int32 GetReserveAmmo(EAmmoType type);
	
    UFUNCTION(BlueprintCallable, Category="Weapon|ADS")
    void StartAiming();

    UFUNCTION(BlueprintCallable, Category="Weapon|ADS")
    void StopAiming();

    UFUNCTION(BlueprintPure, Category="Weapon|ADS")
    bool IsAiming() const { return bIsAiming; }

    // Returns ADS params of currently equipped weapon (fallbacks to component defaults if unavailable)
    UFUNCTION(BlueprintPure, Category="Weapon|ADS")
    const FAimViewParams& GetAimViewParams() const { return WeaponList[CurrentIdx]->GetAimViewParams(); }

private:
    UPROPERTY(VisibleAnywhere, Category="Weapon|ADS")
    bool bIsAiming = false;

    UPROPERTY(EditAnywhere, Category="Weapon|ADS")
    FAimViewParams AimViewParams;
};
