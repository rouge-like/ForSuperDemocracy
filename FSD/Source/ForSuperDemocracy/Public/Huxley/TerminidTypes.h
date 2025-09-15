#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "TerminidTypes.generated.h"

// Terminid 상태 열거형
UENUM(BlueprintType)
enum class ETerminidState : uint8
{
	Idle = 0 UMETA(DisplayName = "Idle"), // 대기 상태 (주변 감시)
	Patrol UMETA(DisplayName = "Patrol"), // 순찰 상태 (지정된 경로 이동)
	Chase UMETA(DisplayName = "Chase"), // 추적 상태 (플레이어 따라가기)
	Attack UMETA(DisplayName = "Attack"), // 공격 상태 (기본 공격 실행)
	Hurt UMETA(DisplayName = "Hurt"), // 피격 상태 (데미지 후 0.5초 스턴)
	Death UMETA(DisplayName = "Death"), // 죽음 상태 (사망 처리)
	Swarm UMETA(DisplayName = "Swarm"), // 군집 상태 (협력 이동)
	Flee UMETA(DisplayName = "Flee"), // 도주 상태 (체력 부족시 도망)
	Burrow UMETA(DisplayName = "Burrow") // 땅속 숨기 상태 (충돌 없음, 감지 대기)
};

// Terminid 타입 열거형
UENUM(BlueprintType)
enum class ETerminidType : uint8
{
	Scavenger = 0 UMETA(DisplayName = "Scavenger"), // 소형, 빠른 속도, 약한 체력
	Warrior UMETA(DisplayName = "Warrior"), // 중형, 균형잡힌 스탯
	Charger UMETA(DisplayName = "Charger") // 대형, 높은 체력, 강한 공격
};

// Terminid 스탯 구조체 (캐시 친화적 16바이트 정렬)
USTRUCT(BlueprintType)
struct FORSUPERDEMOCRACY_API FTerminidStats
{
	GENERATED_BODY()

	// 기본 스탯 (4바이트 * 4 = 16바이트)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Stats")
	float Health = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Stats")
	float MoveSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Stats")
	float AttackDamage = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Stats")
	float AttackCooldown = 1.5f;

	// 범위 스탯 (1바이트 * 4 = 4바이트, 스케일된 값 사용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Range Stats", meta = (ClampMin = "50", ClampMax = "255"))
	uint8 AttackRange = 120; // 실제 범위 (50-255)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Range Stats", meta = (ClampMin = "100", ClampMax = "255"))
	uint8 DetectionRange = 200; // 실제 범위 * 4로 스케일 (400-1020)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
	uint8 TerminidTypeID = 0; // ETerminidType에 대응

	uint8 Reserved = 0; // 패딩용 (향후 확장)

	// 생성자 - 기본값 설정
	FTerminidStats()
	{
		Health = 100.0f;
		MoveSpeed = 300.0f;
		AttackDamage = 25.0f;
		AttackCooldown = 1.5f;
		AttackRange = 120;
		DetectionRange = 200;
		TerminidTypeID = 0;
		Reserved = 0;
	}

	// 타입별 스탯 생성 함수
	static FTerminidStats CreateScavengerStats()
	{
		FTerminidStats Stats;
		Stats.Health = 25.0f;
		Stats.MoveSpeed = 585.0f; // 450 * 1.3 = 30% 증가
		Stats.AttackDamage = 15.0f;
		Stats.AttackCooldown = 1.0f;
		Stats.AttackRange = 100;
		Stats.DetectionRange = 255; // 1020 / 4 - Maximum detection range for Scavenger (uint8 limit)
		Stats.TerminidTypeID = static_cast<uint8>(ETerminidType::Scavenger);
		return Stats;
	}

	static FTerminidStats CreateWarriorStats()
	{
		FTerminidStats Stats;
		Stats.Health = 120.0f;
		Stats.MoveSpeed = 300.0f;
		Stats.AttackDamage = 40.0f;
		Stats.AttackCooldown = 1.5f;
		Stats.AttackRange = 120;
		Stats.DetectionRange = 200; // 800 / 4
		Stats.TerminidTypeID = static_cast<uint8>(ETerminidType::Warrior);
		return Stats;
	}

	static FTerminidStats CreateChargerStats()
	{
		FTerminidStats Stats;
		Stats.Health = 350.0f;
		Stats.MoveSpeed = 200.0f;
		Stats.AttackDamage = 70.0f;
		Stats.AttackCooldown = 2.5f;
		Stats.AttackRange = 150;
		Stats.DetectionRange = 250; // 1000 / 4
		Stats.TerminidTypeID = static_cast<uint8>(ETerminidType::Charger);
		return Stats;
	}

	// 실제 DetectionRange 값 반환 (스케일링 적용)
	FORCEINLINE float GetActualDetectionRange() const
	{
		return static_cast<float>(DetectionRange) * 4.0f;
	}

	// 실제 AttackRange 값 반환
	FORCEINLINE float GetActualAttackRange() const
	{
		return AttackRange;
	}
};

// Terminid 스폰 데이터 구조체ekl
USTRUCT(BlueprintType)
struct FORSUPERDEMOCRACY_API FTerminidSpawnData
{
	GENERATED_BODY()

	// 스폰할 Terminid 타입
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	ETerminidType TerminidType = ETerminidType::Scavenger;

	// 스폰 위치 (스포너 기준 상대적 오프셋)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FVector SpawnOffset = FVector::ZeroVector;

	// 스폰 확률 가중치 (높을수록 자주 스폰)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float SpawnWeight = 1.0f;

	// 이 타입의 최대 활성 개체 수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "1", ClampMax = "50"))
	int32 MaxActiveCount = 5;

	// 스폰 간격 (초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0.5", ClampMax = "30.0"))
	float SpawnInterval = 3.0f;

	FTerminidSpawnData()
	{
		TerminidType = ETerminidType::Scavenger;
		SpawnOffset = FVector::ZeroVector;
		SpawnWeight = 1.0f;
		MaxActiveCount = 5;
		SpawnInterval = 3.0f;
	}
};
