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

public:    
    virtual void Tick(float DeltaTime) override;

    // 스폰 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    TArray<FTerminidSpawnData> SpawnQueue;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0.1", ClampMax = "30.0"))
    float SpawnInterval = 3.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "1", ClampMax = "50"))
    int32 MaxActiveMonsters = 8;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "500.0", ClampMax = "3000.0"))
    float PlayerDetectionRange = 1500.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    bool bSpawnOnlyWhenPlayerNear = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    bool bAutoStartSpawning = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "100.0", ClampMax = "1000.0"))
    float SpawnRadius = 300.0f;

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

    // 플레이어 관련
    UFUNCTION(BlueprintPure, Category = "Spawner")
    APawn* GetNearestPlayer() const;

    UFUNCTION(BlueprintPure, Category = "Spawner")
    float GetDistanceToNearestPlayer() const;

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

public:

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};