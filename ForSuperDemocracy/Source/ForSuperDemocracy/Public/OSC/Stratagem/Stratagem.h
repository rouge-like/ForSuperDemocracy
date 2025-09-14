// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OSC/Weapon/ProjectileBase.h"
#include "Stratagem.generated.h"

class USphereComponent;
class USkeletalMeshComponent;
class UProjectileMovementComponent;
class UAnimSequenceBase;
class UNiagaraSystem;

UCLASS()
class FORSUPERDEMOCRACY_API AStratagem : public AProjectileBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AStratagem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// 딜레이 시간 후 액션 및 자멸
	UPROPERTY(EditDefaultsOnly, Category="Stratagem")
	float DelayTime = 2.0f;

	UPROPERTY(EditDefaultsOnly, Category="Stratagem")
	float Damage = 100.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Stratagem")
	float Radius = 500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Stratagem|Animation")
	TObjectPtr<UAnimSequenceBase> FixingAnim;
	
	UPROPERTY(EditDefaultsOnly, Category = "Stratagem|Animation")
	TObjectPtr<UAnimSequenceBase> FixedAnim;

	UPROPERTY(EditDefaultsOnly, Category="Stratagem|VFX")
	TObjectPtr<UNiagaraSystem> LightVFX;
	
	UPROPERTY(EditDefaultsOnly, Category="Stratagem|VFX")
	TObjectPtr<UNiagaraSystem> ExplosionVFX;
	// 고정 상태 여부 (중복 처리 방지)
	bool bHasLandedAndFixingStarted = false;

	// Fixing 애니메이션 종료 후 호출되는 핸들러
	void OnFixingFinished();

	// ProjectileMovement 정지(충돌) 시 콜백
	UFUNCTION()
	void OnProjectileStopped(const FHitResult& ImpactResult);

	// Fixing 시퀀스 시작
	void StartFixingSequence(const FHitResult& ImpactResult);

	UFUNCTION()
	void AirRaid();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
