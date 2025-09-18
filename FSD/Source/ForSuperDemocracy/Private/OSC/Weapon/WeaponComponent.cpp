// Fill out your copyright notice in the Description page of Project Settings.

#include "OSC/Weapon/WeaponComponent.h"
#include "OSC/Weapon/WeaponBase.h"
#include "Components/ChildActorComponent.h"


// 컴포넌트 기본값 설정(필요 시 Tick 활성화 등)
UWeaponComponent::UWeaponComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// BeginPlay: 오너의 ChildActor 무기 자동 등록 및 초기 장비 설정
void UWeaponComponent::BeginPlay()
{
    Super::BeginPlay();

    // ...
    if (AActor* OwnerActor = GetOwner())
    {
        TArray<UChildActorComponent*> ChildActors;
        OwnerActor->GetComponents<UChildActorComponent>(ChildActors);
        for (UChildActorComponent* CAC : ChildActors)
        {
            if (!CAC) continue;
            if (AActor* Child = CAC->GetChildActor())
            {
                if (AWeaponBase* Weapon = Cast<AWeaponBase>(Child))
                {
                    RegisterWeapon(Weapon);
                }
            }
        }
        if (WeaponList.Num() > 0)
        {
            CurrentIdx = 0;
        }
    }
}


// 매 프레임 호출(필요 시 조준 정렬/카메라 보간 등 확장 가능)
void UWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UWeaponComponent::Equip(int32 idx)
{
    StopFire(); // 전환 전 사격 중지로 안정성 확보
	
	CurrentIdx = idx;
}

void UWeaponComponent::StartFire()
{
    if (!bIsAiming)
        return; // ADS 아닐 경우 사격 금지(게임 디자인 규칙)
	
	WeaponList[CurrentIdx]->StartFire();
}

void UWeaponComponent::StopFire()
{
	WeaponList[CurrentIdx]->StopFire();
}

void UWeaponComponent::Reload()
{
    WeaponList[CurrentIdx]->StartReload(); // 무기 내부 타이머로 완료 콜백 제어
}

int32 UWeaponComponent::PullAmmo(EAmmoType type, int32 need)
{
    int32& pool = AmmoPools.FindOrAdd(type);         // 탄약 풀 생성/조회
    const int32 give = FMath::Clamp(need, 0, pool);  // 요청 수량과 잔량 중 최소만 지급

    pool -= give;

    return give;
}


void UWeaponComponent::RegisterWeapon(AWeaponBase* Weapon)
{
    if (!IsValid(Weapon)) return;
    if (Weapon->GetOwner() != GetOwner())
    {
        Weapon->SetOwner(GetOwner()); // 무기 오너를 컴포넌트 오너로 통일
    }
    
    Weapon->RegisterWeaponComponent(this); // 상호 참조 등록(탄약 풀/상태 공유)
    WeaponList.AddUnique(Weapon);

    Weapon->OnFired.AddUniqueDynamic(this, &UWeaponComponent::HandleWeaponFired);
}

int32 UWeaponComponent::GetReserveAmmo(EAmmoType type)
{
    int32& pool = AmmoPools.FindOrAdd(type);
    return pool;
}

void UWeaponComponent::StartAiming()
{
    bIsAiming = true;

    WeaponList[CurrentIdx]->SetAiming(bIsAiming);
}

void UWeaponComponent::StopAiming()
{
    bIsAiming = false;

    StopFire(); // ADS 해제 시 즉시 사격 중단
    WeaponList[CurrentIdx]->SetAiming(bIsAiming);
}

void UWeaponComponent::HandleWeaponFired(AWeaponBase* Weapon)
{
    OnWeaponFired.Broadcast(Weapon);
}
