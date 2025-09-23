// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HellPod.generated.h"

UCLASS()
class FORSUPERDEMOCRACY_API AHellPod : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AHellPod();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(EditAnywhere)
	class UCapsuleComponent* Collision;

	UPROPERTY(EditAnywhere)
	class USkeletalMeshComponent* Mesh;

	UPROPERTY(EditAnywhere)
	class USpringArmComponent* SpringArm;
	
	UPROPERTY(EditAnywhere)
	class UCameraComponent* Camera;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* Smoke;
	
public:
	UPROPERTY(EditAnywhere, Category = Settings)
	float Speed = 300.f;

	UPROPERTY(EditAnywhere, Category = Settings)
	float DigInTime = 0.1f;
	
protected:
	bool bIsFalling = true;

	UPROPERTY(EditAnywhere, Category = Settings)
	TObjectPtr<USoundBase> HitSound;
	
	UPROPERTY(EditAnywhere, Category = Settings)
	TObjectPtr<USoundAttenuation> SoundAttenuation;

	UFUNCTION()
	void OnSpawnPlayer();
};
