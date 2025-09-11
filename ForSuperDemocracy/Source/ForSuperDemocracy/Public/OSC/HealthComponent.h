// 프로젝트 설정의 설명 페이지에 저작권 공지를 채워주세요.

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
    // 컴포넌트 기본값 설정(체력/사망 가능 여부 등)
    UHealthComponent();

protected:
    // BeginPlay: 체력 초기화 및 데미지 이벤트 바인딩
    virtual void BeginPlay() override;

public:	
    // 매 프레임 호출(필요 시 HUD 동기화 등 확장 가능)
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    
    UFUNCTION(BlueprintCallable, Category="Health")
    void Heal(float Amount); // 체력 회복(상한: MaxHealth)

    UFUNCTION(BlueprintCallable, Category="Health")
    void ApplyDamage(float Amount, AActor* DamageCauser = nullptr, AController* InstigatedBy = nullptr, TSubclassOf<UDamageType> DamageType = nullptr); // 외부에서 데미지 유도(엔진 ApplyDamage 경유)

    UFUNCTION(BlueprintCallable, Category="Health")
    void Kill(); // 즉시 처치(현재 체력만큼 데미지 적용)
 
    UFUNCTION(BlueprintPure, Category="Health")
    bool IsAlive() const { return CurrentHealth > 0.f; } // 생존 여부

    UFUNCTION(BlueprintPure, Category="Health")
    float GetHealthPercent() const { return MaxHealth > 0.f ? CurrentHealth / MaxHealth : 0.f; } // 체력 비율(0~1)

    // 델리게이트 이벤트
    UPROPERTY(BlueprintAssignable, Category="Health|Event")
    FOnHealthChanged OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category="Health|Event")
    FOnDamaged OnDamaged;

    UPROPERTY(BlueprintAssignable, Category="Health|Event")
    FOnDeath OnDeath;

protected:
    // 데미지 이벤트 바인딩 핸들러(엔진 콜백)
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
