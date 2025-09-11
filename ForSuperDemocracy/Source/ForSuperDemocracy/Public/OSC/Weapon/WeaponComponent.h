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
    // 매 프레임 호출(필요 시 갱신 로직 추가)
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(BlueprintReadOnly, Category="Weapon")
    TArray<TObjectPtr<AWeaponBase>> WeaponList; // 장착/소지 중인 무기 목록
	
	UFUNCTION(BlueprintCallable)
    void Equip(int32 idx); // 인덱스 무기 장비 전환(연사 중지 후 교체)

	UFUNCTION(BlueprintCallable)
    void StartFire();   // 사격 시작(ADS 아닐 경우 게이트)

	UFUNCTION(BlueprintCallable)
    void StopFire();    // 사격 종료

	UFUNCTION(BlueprintCallable)
    void Reload();      // 재장전 시작(무기 내부 타이머 기반)

    UFUNCTION()
    int32 PullAmmo(EAmmoType type, int32 need); // 탄약 풀에서 요청 수량 인출

    UFUNCTION(BlueprintCallable, Category="Weapon")
    void RegisterWeapon(AWeaponBase* Weapon);   // ChildActor 무기 자동 등록 등

    UFUNCTION(BlueprintCallable)
    int32 GetReserveAmmo(EAmmoType type);       // 예비 탄약 잔량 조회
	
    UFUNCTION(BlueprintCallable, Category="Weapon|ADS")
    void StartAiming(); // ADS 시작(발사 허용 상태로 전환)

    UFUNCTION(BlueprintCallable, Category="Weapon|ADS")
    void StopAiming();  // ADS 종료(발사 중지, 정렬 복귀 등)

    UFUNCTION(BlueprintPure, Category="Weapon|ADS")
    bool IsAiming() const { return bIsAiming; } // 현재 ADS 상태 여부

    // Returns ADS params of currently equipped weapon (fallbacks to component defaults if unavailable)
    UFUNCTION(BlueprintPure, Category="Weapon|ADS")
    const FAimViewParams& GetAimViewParams() const { return WeaponList[CurrentIdx]->GetAimViewParams(); } // 현재 무기 ADS 파라미터

private:
    UPROPERTY(VisibleAnywhere, Category="Weapon|ADS")
    bool bIsAiming = false; // ADS 상태 플래그(사격 게이팅)

    UPROPERTY(EditAnywhere, Category="Weapon|ADS")
    FAimViewParams AimViewParams; // 컴포넌트 기본 ADS 파라미터(무기 미제공 시 사용)
};
