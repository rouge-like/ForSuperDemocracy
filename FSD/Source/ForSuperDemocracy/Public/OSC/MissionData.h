// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MissionData.generated.h"

// 전방 선언: 헤더 경량화
class AActor;

/**
 * 미션 타입(한 맵에서 순차 수행)
 */
UENUM(BlueprintType)
enum class EMissionType : uint8
{
    Kill        UMETA(DisplayName = "Kill"),
    Destroy     UMETA(DisplayName = "Destroy"),
    ReachArea   UMETA(DisplayName = "ReachArea"),
    Survive     UMETA(DisplayName = "Survive"),
};

/**
 * 단일 목표 스펙
 * - Kill: Target = 처치 수, FilterTag로 대상 필터(옵션)
 * - Destroy: Target = 파괴 개수, FilterTag 또는 TargetClass로 필터
 * - ReachArea: AreaTag 일치 시 1회 완료
 * - Survive: Target = 생존 초(TimeLimit도 사용 가능)
 */
USTRUCT(BlueprintType)
struct FMissionObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
    EMissionType Type = EMissionType::Kill;

    // 목표치(처치 수/파괴 수/생존 초 등)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission", meta=(ClampMin="0"))
    int32 Target = 0;

    // 제한 시간(초) — 선택적 사용(Kill 제한/Survive 등)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission", meta=(ClampMin="0"))
    int32 TimeLimit = 0;

    // 필터 태그: Kill/Destroy 대상 구분용(예: "Nest", "Terminid")
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission|Filter")
    FName FilterTag;

    // Destroy 전용: 특정 클래스만 카운트하고 싶을 때 사용
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission|Filter")
    TSubclassOf<AActor> TargetClass;

    // 영역 기반 목표: 복귀 지점 등 트리거 식별용 태그
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission|Area")
    FName AreaTag;

    // UI 표기용 라벨
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission|UI")
    FText Label;
};

/**
 * 미션 데이터(연속 목표 목록)
 */
UCLASS(BlueprintType)
class FORSUPERDEMOCRACY_API UMissionData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // 한 맵 내에서 순서대로 수행할 목표 리스트
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
    TArray<FMissionObjective> Objectives;
};
