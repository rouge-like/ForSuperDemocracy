#include "Huxley/TerminidSpawner.h"
#include "Huxley/TerminidBase.h"
#include "Huxley/TerminidScavenger.h"
#include "Huxley/TerminidWarrior.h"
#include "Huxley/TerminidCharger.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "NavigationSystem.h"
#include "DrawDebugHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/DamageType.h"

ATerminidSpawner::ATerminidSpawner()
{
	PrimaryActorTick.bCanEverTick = true;

	// 기본값 설정
	SpawnInterval = 3.0f;
	MaxActiveMonsters = 8;
	PlayerDetectionRange = 1500.0f;
	bSpawnOnlyWhenPlayerNear = true;
	bAutoStartSpawning = true;
	SpawnRadius = 300.0f;

	CurrentSpawnIndex = 0;
	LastSpawnTime = 0.0f;
	bIsSpawningActive = false;
	bIsSpawningPaused = false;

	// 파괴 시스템 초기화
	CurrentHealth = MaxHealth;
	bIsDestroyed = false;
	SpawnedBlockingMesh = nullptr;
}

void ATerminidSpawner::BeginPlay()
{
	Super::BeginPlay();

	InitializeSpawner();

	// 자동 시작이 설정되어 있으면 스폰 시작
	if (bAutoStartSpawning)
	{
		StartSpawning();
	}
}

void ATerminidSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 무효한 몬스터들 정리
	CleanupInvalidMonsters();

	// 스폰 시스템 업데이트
	if (bIsSpawningActive && !bIsSpawningPaused)
	{
		UpdateSpawning(DeltaTime);
	}

	// 플레이어 근접성 체크 (1초마다)
	if (bSpawnOnlyWhenPlayerNear)
	{
		CheckPlayerProximity();
	}
}

void ATerminidSpawner::StartSpawning()
{
	bIsSpawningActive = true;
	bIsSpawningPaused = false;
	LastSpawnTime = GetWorld()->GetTimeSeconds();
}

void ATerminidSpawner::StopSpawning()
{
	bIsSpawningActive = false;
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
}

void ATerminidSpawner::PauseSpawning()
{
	bIsSpawningPaused = true;
}

void ATerminidSpawner::ResumeSpawning()
{
	bIsSpawningPaused = false;
}

ATerminidBase* ATerminidSpawner::SpawnTerminid(ETerminidType TerminidType, const FVector& SpawnLocation)
{
	// 클래스 참조 가져오기
	TSubclassOf<ATerminidBase> TerminidClass = GetTerminidClass(TerminidType);
	if (!TerminidClass)
	{
		return nullptr;
	}

	// 스폰 위치 유효성 검사
	if (!IsSpawnLocationValid(SpawnLocation))
	{
		return nullptr;
	}

	// Terminid 스폰
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = nullptr;

	ATerminidBase* NewTerminid = GetWorld()->SpawnActor<ATerminidBase>(
		TerminidClass,
		SpawnLocation,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (NewTerminid)
	{
		// 스탯 설정
		FTerminidStats Stats;
		switch (TerminidType)
		{
		case ETerminidType::Scavenger:
			Stats = FTerminidStats::CreateScavengerStats();
			break;
		case ETerminidType::Warrior:
			Stats = FTerminidStats::CreateWarriorStats();
			break;
		case ETerminidType::Charger:
			Stats = FTerminidStats::CreateChargerStats();
			break;
		}

		// Terminid 초기화
		NewTerminid->InitializeTerminid(Stats, TerminidType);
		NewTerminid->SetParentSpawner(this);

		// 스폰 애니메이션 시작 (AI 비활성화)
		NewTerminid->StartSpawnSequence();

		// 활성 몬스터 목록에 추가
		ActiveMonsters.Add(NewTerminid);
		UpdateActiveMonsterCounts();
	}

	return NewTerminid;
}

void ATerminidSpawner::SpawnNextMonster()
{
	if (!CanSpawnMonster() || SpawnQueue.Num() == 0)
	{
		return;
	}

	// 스폰 큐에서 다음 몬스터 선택
	if (CurrentSpawnIndex >= SpawnQueue.Num())
	{
		CurrentSpawnIndex = 0;
	}

	const FTerminidSpawnData& SpawnData = SpawnQueue[CurrentSpawnIndex];

	// 타입별 최대 개체 수 확인
	int32 CurrentTypeCount = GetActiveMonsterCountOfType(SpawnData.TerminidType);
	if (CurrentTypeCount >= SpawnData.MaxActiveCount)
	{
		CurrentSpawnIndex++;
		return;
	}

	// 스폰 위치 계산
	FVector SpawnLocation = GetRandomSpawnLocation();

	// 몬스터 스폰
	ATerminidBase* NewMonster = SpawnTerminid(SpawnData.TerminidType, SpawnLocation);

	if (NewMonster)
	{
		LastSpawnTime = GetWorld()->GetTimeSeconds();
		CurrentSpawnIndex++;
	}
}

void ATerminidSpawner::SpawnMonsterOfType(ETerminidType TerminidType)
{
	FVector SpawnLocation = GetRandomSpawnLocation();
	SpawnTerminid(TerminidType, SpawnLocation);
}

// 정보 조회 함수들
int32 ATerminidSpawner::GetActiveMonsterCount() const
{
	return ActiveMonsters.Num();
}

int32 ATerminidSpawner::GetActiveMonsterCountOfType(ETerminidType TerminidType) const
{
	const int32* Count = ActiveMonsterCounts.Find(TerminidType);
	return Count ? *Count : 0;
}

bool ATerminidSpawner::CanSpawnMonster() const
{
	// 파괴된 스포너는 스폰할 수 없음
	if (bIsDestroyed)
	{
		return false;
	}

	if (GetActiveMonsterCount() >= MaxActiveMonsters)
	{
		return false;
	}

	if (bSpawnOnlyWhenPlayerNear && !IsPlayerInRange())
	{
		return false;
	}

	return true;
}

bool ATerminidSpawner::IsPlayerInRange() const
{
	APawn* NearestPlayer = GetNearestPlayer();
	if (!NearestPlayer)
	{
		return false;
	}

	float Distance = FVector::Dist(GetActorLocation(), NearestPlayer->GetActorLocation());
	return Distance <= PlayerDetectionRange;
}

bool ATerminidSpawner::IsSpawningActive() const
{
	return bIsSpawningActive && !bIsSpawningPaused;
}

void ATerminidSpawner::OnMonsterDeath(ATerminidBase* DeadMonster)
{
	if (DeadMonster)
	{
		ActiveMonsters.RemoveSingle(DeadMonster);
		UpdateActiveMonsterCounts();
	}
}

void ATerminidSpawner::ClearAllMonsters()
{
	for (ATerminidBase* Monster : ActiveMonsters)
	{
		if (IsValid(Monster))
		{
			Monster->Die();
		}
	}

	ActiveMonsters.Empty();
	ActiveMonsterCounts.Empty();
}

// 스폰 큐 관리
void ATerminidSpawner::AddToSpawnQueue(const FTerminidSpawnData& SpawnData)
{
	SpawnQueue.Add(SpawnData);
}

void ATerminidSpawner::ClearSpawnQueue()
{
	SpawnQueue.Empty();
}

void ATerminidSpawner::ResetSpawnQueue()
{
	CurrentSpawnIndex = 0;
}

// 스폰 위치 계산 (중앙 스폰으로 변경)
FVector ATerminidSpawner::GetRandomSpawnLocation() const
{
	// 스포너 중앙에서 바로 스폰 (애니메이션 시퀀스를 위해)
	FVector SpawnerCenter = GetActorLocation();

	// 지면에 정확히 위치하도록 Z값 조정
	SpawnerCenter.Z += 50.0f; // 스포너 위쪽에서 시작

	return SpawnerCenter;
}

FVector ATerminidSpawner::GetSpawnLocationWithOffset(const FVector& Offset) const
{
	return GetActorLocation() + Offset;
}

// 플레이어 관련
APawn* ATerminidSpawner::GetNearestPlayer() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	APawn* NearestPlayer = nullptr;
	float NearestDistance = FLT_MAX;
	FVector MyLocation = GetActorLocation();

	// 모든 플레이어 컨트롤러를 찾아서 가장 가까운 플레이어 반환
	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PC = Iterator->Get();
		if (PC && PC->GetPawn())
		{
			float Distance = FVector::Dist(MyLocation, PC->GetPawn()->GetActorLocation());
			if (Distance < NearestDistance)
			{
				NearestDistance = Distance;
				NearestPlayer = PC->GetPawn();
			}
		}
	}

	return NearestPlayer;
}

float ATerminidSpawner::GetDistanceToNearestPlayer() const
{
	APawn* NearestPlayer = GetNearestPlayer();
	if (!NearestPlayer)
	{
		return FLT_MAX;
	}

	return FVector::Dist(GetActorLocation(), NearestPlayer->GetActorLocation());
}

// Protected 함수들
void ATerminidSpawner::UpdateSpawning(float DeltaTime)
{
	float CurrentTime = GetWorld()->GetTimeSeconds();

	// 스폰 간격 체크
	if (CurrentTime - LastSpawnTime >= SpawnInterval)
	{
		SpawnNextMonster();
	}
}

bool ATerminidSpawner::IsSpawnLocationValid(const FVector& Location) const
{
	// 기본적인 위치 유효성 검사
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	// 간단한 위치 확인 - NavMesh 체크는 나중에 추가

	return true;
}

ETerminidType ATerminidSpawner::SelectSpawnType() const
{
	// 가중치 기반 타입 선택
	float TotalWeight = 0.0f;
	TArray<float> Weights;

	for (const FTerminidSpawnData& SpawnData : SpawnQueue)
	{
		Weights.Add(SpawnData.SpawnWeight);
		TotalWeight += SpawnData.SpawnWeight;
	}

	if (TotalWeight <= 0.0f || Weights.Num() == 0)
	{
		return ETerminidType::Scavenger;
	}

	float RandomValue = FMath::RandRange(0.0f, TotalWeight);
	float CurrentWeight = 0.0f;

	for (int32 i = 0; i < Weights.Num(); i++)
	{
		CurrentWeight += Weights[i];
		if (RandomValue <= CurrentWeight)
		{
			return SpawnQueue[i].TerminidType;
		}
	}

	return SpawnQueue.Last().TerminidType;
}

TSubclassOf<ATerminidBase> ATerminidSpawner::GetTerminidClass(ETerminidType TerminidType) const
{
	switch (TerminidType)
	{
	case ETerminidType::Scavenger:
		return ScavengerClass;
	case ETerminidType::Warrior:
		return WarriorClass;
	case ETerminidType::Charger:
		return ChargerClass;
	default:
		return ScavengerClass;
	}
}

// Private 함수들
void ATerminidSpawner::InitializeSpawner()
{
	ActiveMonsters.Empty();
	ActiveMonsterCounts.Empty();
	CurrentSpawnIndex = 0;
	LastSpawnTime = 0.0f;

	// 기본 스폰 큐 설정
	if (SpawnQueue.Num() == 0)
	{
		SetupDefaultSpawnQueue();
	}
}

void ATerminidSpawner::UpdateActiveMonsterCounts()
{
	ActiveMonsterCounts.Empty();

	for (ATerminidBase* Monster : ActiveMonsters)
	{
		if (IsValid(Monster))
		{
			ETerminidType Type = Monster->GetTerminidType();
			int32& Count = ActiveMonsterCounts.FindOrAdd(Type);
			Count++;
		}
	}
}

void ATerminidSpawner::CleanupInvalidMonsters()
{
	// 무효하거나 죽은 몬스터들 제거
	ActiveMonsters.RemoveAll([](ATerminidBase* Monster)
	{
		return !IsValid(Monster) || !Monster->IsAlive();
	});

	UpdateActiveMonsterCounts();
}

void ATerminidSpawner::CheckPlayerProximity()
{
	// 플레이어 근접성 체크는 이미 IsPlayerInRange()에서 구현됨
}

void ATerminidSpawner::SetupDefaultSpawnQueue()
{
	// 기본 스캐빈저 스폰 데이터
	FTerminidSpawnData ScavengerData;
	ScavengerData.TerminidType = ETerminidType::Scavenger;
	ScavengerData.SpawnWeight = 7.0f; // 70% 확률
	ScavengerData.MaxActiveCount = 15;
	ScavengerData.SpawnInterval = 2.0f;
	ScavengerData.SpawnOffset = FVector::ZeroVector;
	SpawnQueue.Add(ScavengerData);

	// 기본 워리어 스폰 데이터
	FTerminidSpawnData WarriorData;
	WarriorData.TerminidType = ETerminidType::Warrior;
	WarriorData.SpawnWeight = 2.5f; // 25% 확률
	WarriorData.MaxActiveCount = 5;
	WarriorData.SpawnInterval = 5.0f;
	WarriorData.SpawnOffset = FVector::ZeroVector;
	SpawnQueue.Add(WarriorData);

	// 기본 차저 스폰 데이터
	FTerminidSpawnData ChargerData;
	ChargerData.TerminidType = ETerminidType::Charger;
	ChargerData.SpawnWeight = 0.5f; // 5% 확률
	ChargerData.MaxActiveCount = 1;
	ChargerData.SpawnInterval = 10.0f;
	ChargerData.SpawnOffset = FVector::ZeroVector;
	SpawnQueue.Add(ChargerData);
}

// 파괴 시스템 구현
float ATerminidSpawner::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser)
{
	// 이미 파괴된 경우 데미지 무시
	if (bIsDestroyed)
	{
		return 0.0f;
	}

	// 데미지 적용
	CurrentHealth = FMath::Max(0.0f, CurrentHealth - DamageAmount);

	// 디버그 프린트: 데미지 받음
	UE_LOG(LogTemp, Warning, TEXT("Spawner Hit! Damage: %.1f, Health: %.1f/%.1f"), DamageAmount, CurrentHealth, MaxHealth);

	// 폭발 이팩트 이벤트 호출 (Blueprint에서 구현)
	OnExplosionHit(GetActorLocation(), DamageAmount);

	// 체력이 0 이하가 되면 파괴
	if (CurrentHealth <= 0.0f)
	{
		DestroySpawner();
	}

	return DamageAmount;
}

void ATerminidSpawner::DestroySpawner()
{
	if (bIsDestroyed)
	{
		return;
	}

	// 파괴 상태로 전환
	bIsDestroyed = true;
	CurrentHealth = 0.0f;

	// 디버그 프린트: 스포너 파괴됨
	UE_LOG(LogTemp, Error, TEXT("SPAWNER DESTROYED! Stopping all spawning and creating blocking mesh"));

	// 스폰 중지
	StopSpawning();

	// 모든 몬스터 정리
	ClearAllMonsters();

	// 블로킹 메시 생성
	CreateBlockingMesh();

	// 파괴 이벤트 호출 (Blueprint에서 구현)
	OnSpawnerDestroyed();
}

float ATerminidSpawner::GetHealthPercent() const
{
	if (MaxHealth <= 0.0f)
	{
		return 0.0f;
	}
	return CurrentHealth / MaxHealth;
}

void ATerminidSpawner::CreateBlockingMesh()
{
	if (!BlockingMesh || SpawnedBlockingMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot create blocking mesh: %s"),
			!BlockingMesh ? TEXT("No BlockingMesh assigned") : TEXT("Already created"));
		return; // 메시가 없거나 이미 생성된 경우
	}

	// 스태틱 메시 컴포넌트 생성
	SpawnedBlockingMesh = NewObject<UStaticMeshComponent>(this);
	SpawnedBlockingMesh->SetupAttachment(RootComponent);
	SpawnedBlockingMesh->SetStaticMesh(BlockingMesh);

	// 스폰 지점을 막도록 위치 설정
	SpawnedBlockingMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	SpawnedBlockingMesh->SetRelativeRotation(FRotator::ZeroRotator);

	// 충돌 설정 (플레이어와 몬스터가 통과할 수 없도록)
	SpawnedBlockingMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SpawnedBlockingMesh->SetCollisionResponseToAllChannels(ECR_Block);

	// 컴포넌트 등록
	SpawnedBlockingMesh->RegisterComponent();

	// 디버그 프린트: 블로킹 메시 생성 완료
	UE_LOG(LogTemp, Warning, TEXT("Blocking mesh created successfully! Spawn entrance blocked."));
}

#if WITH_EDITOR
void ATerminidSpawner::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// 에디터에서 속성 변경 시 처리
	if (PropertyChangedEvent.Property)
	{
		FName PropertyName = PropertyChangedEvent.Property->GetFName();

		if (PropertyName == GET_MEMBER_NAME_CHECKED(ATerminidSpawner, SpawnQueue))
		{
			// 스폰 큐 변경 시 인덱스 리셋
			CurrentSpawnIndex = 0;
		}
	}
}
#endif
