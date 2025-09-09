#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "TerminidTypes.generated.h"


// Terminid Difinition, Enum & Struct 

// Terminid Type
UENUM(BlueprintType)
enum class ETerminidType : uint8
{
	Warrior UMETA(DisplayName = "Warrior"),
	Charger UMETA(DisplayName = "Charger"),
	Titan UMETA(DisplayName = "Titan")
};

// AI State
UENUM(BlueprintType)
enum class ETerminidState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Patrolling UMETA(DisplayName = "Patrolling"),
	Chasing UMETA(DisplayName = "Chasing"),
	Attacking UMETA(DisplayName = "Attacking"),
	Damaged UMETA(DisplayName = "Damaged"),
	Dead UMETA(DisplayName = "Dead")
};

// Attack Type
UENUM(BlueprintType)
enum class EAttackType : uint8
{
	Normal UMETA(DisplayName = "Normal"),
	Melee UMETA(DisplayName = "Melee"), // 근접 공격
	Charge UMETA(DisplayName = "Charge"), // 돌진 공격  
	Ranged UMETA(DisplayName = "Ranged") // 원거리 공격
};

// Team ID
USTRUCT(BlueprintType)
struct FTerminidTeams
{
	GENERATED_BODY()

	static constexpr int32 PlayerTeam = 0; // 플레이어
	static constexpr int32 EnemyTeam = 1; // 테르미니드
	static constexpr int32 NeutralTeam = 255; // 중립
};

// Base stat Struct
USTRUCT(BlueprintType)
struct FTerminidStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float AttackDamage = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float AttackRange = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MovementSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float AttackCooldown = 2.0f;

	// Ctor
	FTerminidStats()
	{
		MaxHealth = 100.0f;
		AttackDamage = 25.0f;
		AttackRange = 150.0f;
		MovementSpeed = 300.0f;
		AttackCooldown = 2.0f;
	}
};

// AI Setting Struct
USTRUCT(BlueprintType)
struct FTerminidAIConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float SightRadius = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float LoseSightRadius = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float FieldOfView = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float HearingRange = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float PatrolRadius = 500.0f;

	// Ctor
	FTerminidAIConfig()
	{
		SightRadius = 1000.0f;
		LoseSightRadius = 1200.0f;
		FieldOfView = 90.0f;
		HearingRange = 800.0f;
		PatrolRadius = 500.0f;
	}
};
