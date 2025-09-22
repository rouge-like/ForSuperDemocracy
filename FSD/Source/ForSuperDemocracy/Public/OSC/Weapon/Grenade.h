// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectileBase.h"
#include "GameFramework/Actor.h"
#include "Grenade.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UNiagaraSystem;

UCLASS()
class FORSUPERDEMOCRACY_API AGrenade : public AProjectileBase
{
	GENERATED_BODY()
	
public:	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	

    // 퓨즈 시간(초) 후 폭발 FX 스폰 및 자멸
    UPROPERTY(EditDefaultsOnly, Category="Grenade")
    float FuseTime = 2.0f;
	// 데미지
	UPROPERTY(EditDefaultsOnly, Category="Grenade")
	float Damage = 2.0f;
	// 폭발 범위
	UPROPERTY(EditDefaultsOnly, Category="Grenade")
	float Radius = 2.0f;
	// 미는 힘
	UPROPERTY(EditDefaultsOnly, Category="Grenade")
	float Power = 2.0f;
	// 폭발 이펙트
	UPROPERTY(EditDefaultsOnly, Category="Grenade")
	TObjectPtr<UNiagaraSystem> ExplosionVFX;
	
	UPROPERTY(EditDefaultsOnly, Category="Grenade")
	TObjectPtr<USoundBase> ExplosionSFX;
	
	UPROPERTY(EditDefaultsOnly, Category="Grenade")
	TObjectPtr<USoundAttenuation> Attenuation;
    UFUNCTION()
    void Explode();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
