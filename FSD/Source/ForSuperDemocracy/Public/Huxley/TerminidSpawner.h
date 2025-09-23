#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "TerminidTypes.h"
#include "TerminidSpawner.generated.h"

class ATerminidBase;
class ATerminidScavenger;
class ATerminidWarrior;
class ATerminidCharger;

UCLASS()
class FORSUPERDEMOCRACY_API ATerminidSpawner : public AActor
{
	GENERATED_BODY()

public:
	ATerminidSpawner();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void Tick(float DeltaTime) override;

	// 스폰 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TArray<FTerminidSpawnData> SpawnQueue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0.1", ClampMax = "30.0"))
	float SpawnInterval = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "1", ClampMax = "50"))
	int32 MaxActiveMonsters = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "500.0", ClampMax = "30000.0"))
	float PlayerDetectionRange = 8000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	bool bSpawnOnlyWhenPlayerNear = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	bool bAutoStartSpawning = true;
	
	// Terminid 클래스 참조들
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terminid Classes", meta = (AllowedClasses = "TerminidBase"))
	TSubclassOf<ATerminidBase> ScavengerClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terminid Classes", meta = (AllowedClasses = "TerminidBase"))
	TSubclassOf<ATerminidBase> WarriorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terminid Classes", meta = (AllowedClasses = "TerminidBase"))
	TSubclassOf<ATerminidBase> ChargerClass;

	// 스폰 제어 함수들
	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void StartSpawning();

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void StopSpawning();

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void PauseSpawning();

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void ResumeSpawning();

	// 수동 스폰 함수들
	UFUNCTION(BlueprintCallable, Category = "Spawner")
	ATerminidBase* SpawnTerminid(ETerminidType TerminidType, const FVector& SpawnLocation);

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void SpawnNextMonster();

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void SpawnMonsterOfType(ETerminidType TerminidType);

	// 정보 조회 함수들
	UFUNCTION(BlueprintPure, Category = "Spawner")
	int32 GetActiveMonsterCount() const;

	UFUNCTION(BlueprintPure, Category = "Spawner")
	int32 GetActiveMonsterCountOfType(ETerminidType TerminidType) const;

	UFUNCTION(BlueprintPure, Category = "Spawner")
	bool CanSpawnMonster() const;

	UFUNCTION(BlueprintPure, Category = "Spawner")
	bool IsPlayerInRange() const;

	UFUNCTION(BlueprintPure, Category = "Spawner")
	bool IsSpawningActive() const;

	// 몬스터 관리 함수들
	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void OnMonsterDeath(ATerminidBase* DeadMonster);

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void ClearAllMonsters();

	// 스폰 큐 관리
	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void AddToSpawnQueue(const FTerminidSpawnData& SpawnData);

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void ClearSpawnQueue();

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void ResetSpawnQueue();

	// 스폰 위치 계산
	UFUNCTION(BlueprintCallable, Category = "Spawner")
	FVector GetRandomSpawnLocation() const;

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	FVector GetSpawnLocationWithOffset(const FVector& Offset) const;

	// SpawnPoint 관련 함수들
	UFUNCTION(BlueprintCallable, Category = "Spawner")
	FVector GetSpawnPointLocation() const;

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	FRotator GetSpawnPointRotation() const;

	// 플레이어 관련
	UFUNCTION(BlueprintPure, Category = "Spawner")
	APawn* GetNearestPlayer() const;

	UFUNCTION(BlueprintPure, Category = "Spawner")
	float GetDistanceToNearestPlayer() const;

	// 스포너 파괴 시스템
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction", meta = (ClampMin = "50.0", ClampMax = "500.0"))
	float MaxHealth = 150.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Destruction")
	float CurrentHealth;

	UPROPERTY(BlueprintReadOnly, Category = "Destruction")
	bool bIsDestroyed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
	TSubclassOf<class UStaticMeshComponent> BlockingMeshClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
	UStaticMesh* BlockingMesh;

	// 기본 메시 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	UStaticMeshComponent* NormalMeshComponent;

	// 파괴된 메시 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	UStaticMeshComponent* DestroyedMeshComponent;

	// 스폰 포인트 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn")
	class USceneComponent* SpawnPoint;

	// HealthComponent 추가 (OSC 시스템과 연동)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	class UHealthComponent* Health;

	//파괴 관련 함수들 - TakeDamage 오버라이드로 폭발 데미지만 허용
	UFUNCTION(BlueprintCallable, Category = "Destruction")
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable, Category = "Destruction")
	void DestroySpawner();

	UFUNCTION(BlueprintPure, Category = "Destruction")
	bool IsDestroyed() const { return bIsDestroyed; }

	UFUNCTION(BlueprintPure, Category = "Destruction")
	float GetHealthPercent() const;

	// Blueprint 이벤트들 (폭발 이팩트용)
	UFUNCTION(BlueprintImplementableEvent, Category = "Destruction")
	void OnExplosionHit(FVector HitLocation, float DamageAmount);

	UFUNCTION(BlueprintImplementableEvent, Category = "Destruction")
	void OnSpawnerDestroyed();

	// HealthComponent 이벤트 핸들러
	UFUNCTION()
	void OnHealthComponentDeath(AActor* Victim);

	UFUNCTION()
	void OnDamaged(float Damage, AActor* DamageCauser, AController* EventInstigator, TSubclassOf<UDamageType> DamageType);


protected:
	// 스폰 시스템 업데이트
	void UpdateSpawning(float DeltaTime);

	// 스폰 위치 유효성 검사
	bool IsSpawnLocationValid(const FVector& Location) const;

	// 스폰 타입 선택 (가중치 기반)
	ETerminidType SelectSpawnType() const;

	// 클래스 참조 가져오기
	TSubclassOf<ATerminidBase> GetTerminidClass(ETerminidType TerminidType) const;

private:
	// 활성 몬스터 목록
	UPROPERTY()
	TArray<ATerminidBase*> ActiveMonsters;

	// 타입별 활성 몬스터 카운트
	TMap<ETerminidType, int32> ActiveMonsterCounts;

	// 스폰 제어 변수들
	int32 CurrentSpawnIndex;
	float LastSpawnTime;
	bool bIsSpawningActive;
	bool bIsSpawningPaused;

	// 타이머 핸들들
	FTimerHandle SpawnTimerHandle;
	FTimerHandle PlayerCheckTimerHandle;

	// 내부 함수들
	void InitializeSpawner();
	void UpdateActiveMonsterCounts();
	void CleanupInvalidMonsters();
	void CheckPlayerProximity();

	// 스폰 큐 기본값 설정
	void SetupDefaultSpawnQueue();

	// 파괴 시스템 관련
	void CreateBlockingMesh();
	void SwitchToDestroyedMesh();

	// 블로킹 메시 컴포넌트
	UPROPERTY()
	UStaticMeshComponent* SpawnedBlockingMesh;

public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
