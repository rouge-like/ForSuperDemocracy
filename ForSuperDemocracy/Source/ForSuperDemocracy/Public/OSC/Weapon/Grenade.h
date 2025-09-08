// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grenade.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UParticleSystem;

UCLASS()
class FORSUPERDEMOCRACY_API AGrenade : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGrenade();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere)
    TObjectPtr<USphereComponent> SphereComp;
    UPROPERTY(EditAnywhere)
    TObjectPtr<USkeletalMeshComponent> MeshComp;
    UPROPERTY(EditAnywhere)
    TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

    // 퓨즈 시간(초) 후 폭발 FX 스폰 및 자멸
    UPROPERTY(EditDefaultsOnly, Category="Grenade")
    float FuseTime = 2.0f;

    // 폭발 이펙트(파티클 시스템)
    UPROPERTY(EditDefaultsOnly, Category="Grenade")
    TObjectPtr<UParticleSystem> ExplosionFX;

    UFUNCTION()
    void Explode();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
