// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MainMode.generated.h"

/**
 * 
 */
class UMissionComponent;

UCLASS()
class FORSUPERDEMOCRACY_API AMainMode : public AGameModeBase
{
	GENERATED_BODY()
	AMainMode();
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UMissionComponent> MissionComponent;

public:
	UMissionComponent* GetMissionComponent(){ return MissionComponent; };
};
