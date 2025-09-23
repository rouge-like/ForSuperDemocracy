// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MainHUD.generated.h"

class UMainUI;
class UUserWidget;
struct FMissionObjective;
/**
 * 
 */
UCLASS()
class FORSUPERDEMOCRACY_API AMainHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	// WBP_MainUI를 할당할 클래스
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UMainUI> MainUIClass;

	// 생성된 위젯 인스턴스
	UPROPERTY()
	TObjectPtr<UMainUI> MainUI;
	
	// WBP_MainUI를 할당할 클래스
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> CinematicUIClass;

	// 생성된 위젯 인스턴스
	UPROPERTY()
	TObjectPtr<UUserWidget> CinematicUI;

public:
	UFUNCTION(BlueprintCallable)
	UMainUI* GetMainUI(){ return MainUI; }

	UFUNCTION()
	void OnMissionObjectiveChanged(const FMissionObjective& Objective);
	UFUNCTION()
	void OnMissionObjectiveUpdate(int32 Curr, int32 Target);
	UFUNCTION()
	void OnMissionComplete();
	UFUNCTION()
	void OnMissionTimerTick(int32 RemainSec);
};
