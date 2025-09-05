// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

class AController;
class UDamageType;
struct FHitResult;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, NewHealth, float, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnDamaged, float, Damage, AActor*, DamageCauser, AController*, InstigatedBy, TSubclassOf<UDamageType>, DamageType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, Victim);



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FORSUPERDEMOCRACY_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
    // Sets default values for this component's properties
    UHealthComponent();

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

public:	
    // Called every frame (optional)
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    
    UFUNCTION(BlueprintCallable, Category="Health")
    void Heal(float Amount);

    UFUNCTION(BlueprintCallable, Category="Health")
    void ApplyDamage(float Amount, AActor* DamageCauser = nullptr, AController* InstigatedBy = nullptr, TSubclassOf<UDamageType> DamageType = nullptr);

    UFUNCTION(BlueprintCallable, Category="Health")
    void Kill();
 
    UFUNCTION(BlueprintPure, Category="Health")
    bool IsAlive() const { return CurrentHealth > 0.f; }

    UFUNCTION(BlueprintPure, Category="Health")
    float GetHealthPercent() const { return MaxHealth > 0.f ? CurrentHealth / MaxHealth : 0.f; }

    // Events
    UPROPERTY(BlueprintAssignable, Category="Health|Event")
    FOnHealthChanged OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category="Health|Event")
    FOnDamaged OnDamaged;

    UPROPERTY(BlueprintAssignable, Category="Health|Event")
    FOnDeath OnDeath;

protected:
    // Damage event bindings
    UFUNCTION()
    void HandleAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

    UFUNCTION()
    void HandlePointDamage(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser);

    UFUNCTION()
    void HandleRadialDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, FVector Origin, const FHitResult& HitInfo, AController* InstigatedBy, AActor* DamageCauser);
    
    void ApplyDamageInternal(float Damage, AActor* DamageCauser, AController* InstigatedBy, const UDamageType* DamageType);
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Health")
    float MaxHealth = 100.f;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Health")
    float CurrentHealth = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Health")
    bool bCanDie = true;
};
