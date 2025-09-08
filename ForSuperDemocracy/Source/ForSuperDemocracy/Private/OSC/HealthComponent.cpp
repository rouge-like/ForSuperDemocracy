// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/HealthComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/EngineTypes.h"

// 컴포넌트 기본값 설정(틱 활성화 등)
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// BeginPlay: 체력 초기화 및 데미지 콜백 연결
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


// 매 프레임 호출(필요 시 HUD 동기화 등 확장 가능)
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

// 외부에서 데미지 유도 시 사용(엔진 ApplyDamage를 통해 내부 이벤트 흐름으로 연결)
void UHealthComponent::ApplyDamage(float Amount, AActor* DamageCauser, AController* InstigatedBy, TSubclassOf<UDamageType> DamageType)
{
    if (Amount <= 0.f)
        return;

    if (AActor* Owner = GetOwner())
    {
        UGameplayStatics::ApplyDamage(Owner, Amount, InstigatedBy, DamageCauser, DamageType);
    }
}

// 현재 체력만큼 내부 데미지 처리 → 즉시 사망 유도
void UHealthComponent::Kill()
{
    if (CurrentHealth <= 0.f)
        return;

    ApplyDamageInternal(CurrentHealth, /*DamageCauser*/nullptr, /*InstigatedBy*/nullptr, /*DamageType*/nullptr);
}

// 엔진 AnyDamage 콜백 → 내부 공통 처리로 위임
void UHealthComponent::HandleAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
    ApplyDamageInternal(Damage, DamageCauser, InstigatedBy, DamageType);
}

// 포인트 데미지(탄환 등) 콜백 → 내부 공통 처리로 위임
void UHealthComponent::HandlePointDamage(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser)
{
    ApplyDamageInternal(Damage, DamageCauser, InstigatedBy, DamageType);
}

// 반경 데미지(폭발 등) 콜백 → 내부 공통 처리로 위임
void UHealthComponent::HandleRadialDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, FVector Origin, const FHitResult& HitInfo, AController* InstigatedBy, AActor* DamageCauser)
{
    ApplyDamageInternal(Damage, DamageCauser, InstigatedBy, DamageType);
}

// 공통 데미지 처리: 체력 감소, 이벤트 브로드캐스트, 사망 처리
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

    if(GEngine) GEngine->AddOnScreenDebugMessage(2, 1.5f, FColor::Green, FString::Printf(TEXT("%s HP %.f / %.f"), *GetOwner()->GetActorNameOrLabel(), CurrentHealth, MaxHealth)); // 디버그: 현재 HP 출력
}
