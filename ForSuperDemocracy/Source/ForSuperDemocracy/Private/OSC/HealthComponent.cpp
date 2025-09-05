// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/HealthComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/EngineTypes.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth;

    if (AActor* Owner = GetOwner())
    {
        Owner->SetCanBeDamaged(true);
        Owner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::HandleAnyDamage);
        Owner->OnTakePointDamage.AddDynamic(this, &UHealthComponent::HandlePointDamage);
        Owner->OnTakeRadialDamage.AddDynamic(this, &UHealthComponent::HandleRadialDamage);
    }
}


// Called every frame
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UHealthComponent::Heal(float Amount)
{
    if (Amount <= 0.f || CurrentHealth <= 0.f)
        return;

    const float Old = CurrentHealth;
    CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.f, MaxHealth);
    OnHealthChanged.Broadcast(CurrentHealth, CurrentHealth - Old);
}

void UHealthComponent::ApplyDamage(float Amount, AActor* DamageCauser, AController* InstigatedBy, TSubclassOf<UDamageType> DamageType)
{
    if (Amount <= 0.f)
        return;

    if (AActor* Owner = GetOwner())
    {
        UGameplayStatics::ApplyDamage(Owner, Amount, InstigatedBy, DamageCauser, DamageType);
    }
}

void UHealthComponent::Kill()
{
    if (CurrentHealth <= 0.f)
        return;

    ApplyDamageInternal(CurrentHealth, /*DamageCauser*/nullptr, /*InstigatedBy*/nullptr, /*DamageType*/nullptr);
}

void UHealthComponent::HandleAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
    ApplyDamageInternal(Damage, DamageCauser, InstigatedBy, DamageType);
}

void UHealthComponent::HandlePointDamage(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser)
{
    ApplyDamageInternal(Damage, DamageCauser, InstigatedBy, DamageType);
}

void UHealthComponent::HandleRadialDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, FVector Origin, const FHitResult& HitInfo, AController* InstigatedBy, AActor* DamageCauser)
{
    ApplyDamageInternal(Damage, DamageCauser, InstigatedBy, DamageType);
}

void UHealthComponent::ApplyDamageInternal(float Damage, AActor* DamageCauser, AController* InstigatedBy, const UDamageType* DamageType)
{
    if (Damage <= 0.f)
        return;

    if (CurrentHealth <= 0.f)
        return;

    const float Old = CurrentHealth;
    CurrentHealth = FMath::Clamp(Old - Damage, 0.f, MaxHealth);

    OnDamaged.Broadcast(Damage, DamageCauser, InstigatedBy, DamageType ? DamageType->GetClass() : nullptr);
    OnHealthChanged.Broadcast(CurrentHealth, CurrentHealth - Old);

    if (CurrentHealth <= 0.f && bCanDie)
    {
        OnDeath.Broadcast(GetOwner());
    }

    if(GEngine) GEngine->AddOnScreenDebugMessage(2, 1.5f, FColor::Green, FString::Printf(TEXT("%s HP %.f / %.f"), *GetOwner()->GetActorNameOrLabel(), MaxHealth, CurrentHealth));
}

