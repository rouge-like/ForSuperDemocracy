// 프로젝트 설정의 설명 페이지에 저작권 공지를 채워주세요.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

class AController;
class UDamageType;
struct FHitResult;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, MaxHealth, float, CurrentHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnDamaged, float, Damage, AActor*, DamageCauser, AController*, InstigatedBy, TSubclassOf<UDamageType>, DamageType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, Victim);
// 래그돌 상태 이벤트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRagdollStart, AActor*, OwnerActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRagdollEnd, AActor*, OwnerActor);

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

    // Ragdoll events for external listeners (e.g., Player camera)
    UPROPERTY(BlueprintAssignable, Category="Health|Ragdoll|Event")
    FOnRagdollStart OnRagdollStart;

    UPROPERTY(BlueprintAssignable, Category="Health|Ragdoll|Event")
    FOnRagdollEnd OnRagdollEnd;

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

    // Ragdoll impulse tuning: impulse strength per 1 damage
    UPROPERTY(EditDefaultsOnly, Category="Health|Ragdoll", meta=(ClampMin="0"))
    float ImpulsePerDamage = 10.f;

    // Time before exiting simple ragdoll
    UPROPERTY(EditDefaultsOnly, Category="Health|Ragdoll", meta=(ClampMin="0"))
    float RagdollRecoverTime = 1.5f;

    // lightweight state for simple ragdoll flow
    UPROPERTY(Transient)
    bool bIsRagdolling = false;

    UPROPERTY(Transient)
    FName PrevMeshCollisionProfileName;
    
    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<UParticleSystem> HitParticle; 
    
    // Store gravity scale to restore after ragdoll
    UPROPERTY(Transient)
    float PrevGravityScale = 1.0f;

    UFUNCTION()
    void RecoverFromRagdoll();

    // Capsule ↔ Mesh sync while ragdolled (for camera/actor cohesion)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Health|Ragdoll")
    bool bSyncCapsuleToRagdoll = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Health|Ragdoll")
    FName RagdollPelvisBoneName = TEXT("pelvis");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Health|Ragdoll", meta=(ClampMin="0"))
    float RagdollCapsuleFollowZOffset = 0.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Health|Ragdoll")
    bool bSyncCapsuleYawToPelvis = false;

    void UpdateCapsuleFollowRagdoll(float DeltaTime);

    // Detach/reattach strategy to avoid parent transform jitter
    UPROPERTY(Transient)
    bool bDetachedMeshDuringRagdoll = false;

    UPROPERTY(Transient)
    FTransform SavedMeshRelativeTransform;

public:
    void ResetHealth();
};
