// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponComponent.generated.h"

class AWeaponBase;
class UWeaponData;
enum class EAmmoType : uint8;
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
	
	int32 currentIdx;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ammo")
	TMap<EAmmoType, int32> AmmoPools;
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(BlueprintReadOnly, Category="Weapon")
    TArray<TObjectPtr<AWeaponBase>> weaponList;
	
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
};
