#include "Huxley/TerminidSpawner.h"
#include "Huxley/TerminidBase.h"
#include "Huxley/TerminidScavenger.h"
#include "Huxley/TerminidWarrior.h"
#include "Huxley/TerminidCharger.h"
#include "OSC/HealthComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "NavigationSystem.h"
#include "DrawDebugHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/DamageType.h"
#include "Engine/DamageEvents.h"

ATerminidSpawner::ATerminidSpawner()
{
	PrimaryActorTick.bCanEverTick = true;

	// 메시 컴포넌트들 생성
	NormalMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NormalMesh"));
	RootComponent = NormalMeshComponent;

	DestroyedMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DestroyedMesh"));
	DestroyedMeshComponent->SetupAttachment(RootComponent);

	// 초기에는 파괴된 메시는 숨기고 콜리전도 비활성화
	DestroyedMeshComponent->SetVisibility(false);
	DestroyedMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 스폰 포인트 컴포넌트 생성
	SpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnPoint"));
	SpawnPoint->SetupAttachment(RootComponent);

	// 스폰 포인트 기본 위치 설정 (스포너 앞쪽으로 약간 떨어진 위치)
	SpawnPoint->SetRelativeLocation(FVector(200.0f, 0.0f, 0.0f));
	SpawnPoint->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

	// 기본값 설정
	SpawnInterval = 3.0f;
	MaxActiveMonsters = 8;
	PlayerDetectionRange = 8000.0f;
	bSpawnOnlyWhenPlayerNear = true;
	bAutoStartSpawning = true;

	CurrentSpawnIndex = 0;
	LastSpawnTime = 0.0f;
	bIsSpawningActive = false;
	bIsSpawningPaused = false;

	// 파괴 시스템 초기화
	CurrentHealth = MaxHealth;
	bIsDestroyed = false;
	SpawnedBlockingMesh = nullptr;

	// HealthComponent 생성
	Health = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

void ATerminidSpawner::BeginPlay()
{
	Super::BeginPlay();

	// HealthComponent 이벤트 바인딩
	if (Health)
	{
		Health->OnDamaged.AddDynamic(this, &ATerminidSpawner::OnDamaged);
		Health->OnDeath.AddDynamic(this, &ATerminidSpawner::OnHealthComponentDeath);
	}

	InitializeSpawner();

	// 자동 시작이 설정되어 있으면 스폰 시작
	if (bAutoStartSpawning)
	{
		StartSpawning();
	}
}

void ATerminidSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 스폰 중지
	StopSpawning();

	// 모든 타이머 정리
	if (UWorld* World = GetWorld())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(SpawnTimerHandle);
		TimerManager.ClearTimer(PlayerCheckTimerHandle);
	}

	// HealthComponent 이벤트 언바인딩
	if (Health)
	{
		Health->OnDamaged.RemoveAll(this);
		Health->OnDeath.RemoveAll(this);
	}

	// 활성 몬스터 목록 안전하게 정리
	for (int32 i = ActiveMonsters.Num() - 1; i >= 0; --i)
	{
		if (ActiveMonsters.IsValidIndex(i) && IsValid(ActiveMonsters[i]))
		{
			ATerminidBase* Monster = ActiveMonsters[i];
			if (Monster)
			{
				Monster->SetParentSpawner(nullptr);
			}
		}
	}
	ActiveMonsters.Empty();

	// 컨테이너들 안전하게 정리
	ActiveMonsterCounts.Empty();
	SpawnQueue.Empty();

	Super::EndPlay(EndPlayReason);
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

	// 안전하게 타이머 정리
	if (UWorld* World = GetWorld())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		if (SpawnTimerHandle.IsValid())
		{
			TimerManager.ClearTimer(SpawnTimerHandle);
		}
		if (PlayerCheckTimerHandle.IsValid())
		{
			TimerManager.ClearTimer(PlayerCheckTimerHandle);
		}
	}
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
	// if (!IsSpawnLocationValid(SpawnLocation))
	// {
	// 	return nullptr;
	// }

	// SpawnPoint의 회전값 가져오기
	FRotator SpawnRotation = GetSpawnPointRotation();

	// Terminid 스폰
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = nullptr;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ATerminidBase* NewTerminid = GetWorld()->SpawnActor<ATerminidBase>(
		TerminidClass,
		SpawnLocation,
		SpawnRotation,
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

		// 스포너에서 생성된 터미니드로 표시
		NewTerminid->bIsSpawnedBySpawner = true;

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
	if (IsValid(DeadMonster))
	{
		// 안전하게 제거
		int32 RemovedCount = ActiveMonsters.RemoveSingle(DeadMonster);
		if (RemovedCount > 0)
		{
			UpdateActiveMonsterCounts();
		}

		// 부모 스포너 참조 해제
		DeadMonster->SetParentSpawner(nullptr);
	}
}

void ATerminidSpawner::ClearAllMonsters()
{
	// 안전하게 모든 몬스터 정리
	for (int32 i = ActiveMonsters.Num() - 1; i >= 0; --i)
	{
		if (ActiveMonsters.IsValidIndex(i))
		{
			ATerminidBase* Monster = ActiveMonsters[i];
			if (IsValid(Monster) && Monster->IsValidLowLevel())
			{
				Monster->SetParentSpawner(nullptr);
				Monster->Die();
			}
		}
	}

	ActiveMonsters.Empty();
	ActiveMonsterCounts.Reset();
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

// 스폰 위치 계산 (SpawnPoint 기반으로 변경)
FVector ATerminidSpawner::GetRandomSpawnLocation() const
{
	if (!SpawnPoint)
	{
		// SpawnPoint가 없으면 기본 위치 반환
		return GetActorLocation();
	}

	// SpawnPoint의 월드 위치 반환
	return SpawnPoint->GetComponentLocation();
}

FVector ATerminidSpawner::GetSpawnLocationWithOffset(const FVector& Offset) const
{
	return GetSpawnPointLocation() + Offset;
}

FVector ATerminidSpawner::GetSpawnPointLocation() const
{
	if (!SpawnPoint)
	{
		return GetActorLocation();
	}
	return SpawnPoint->GetComponentLocation();
}

FRotator ATerminidSpawner::GetSpawnPointRotation() const
{
	if (!SpawnPoint)
	{
		return GetActorRotation();
	}
	return SpawnPoint->GetComponentRotation();
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
	// 무조건 스폰 허용
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
	// 안전하게 초기화
	ActiveMonsters.Empty();
	ActiveMonsterCounts.Reset();
	CurrentSpawnIndex = 0;
	LastSpawnTime = 0.0f;
	bIsSpawningActive = false;
	bIsSpawningPaused = false;

	// 타이머 핸들 초기화
	SpawnTimerHandle.Invalidate();
	PlayerCheckTimerHandle.Invalidate();

	// 기본 스폰 큐 설정
	if (SpawnQueue.Num() == 0)
	{
		SetupDefaultSpawnQueue();
	}
}

void ATerminidSpawner::UpdateActiveMonsterCounts()
{
	// 안전하게 컨테이너 정리
	ActiveMonsterCounts.Reset();

	// 유효한 몬스터들만 카운트
	for (int32 i = 0; i < ActiveMonsters.Num(); ++i)
	{
		if (ActiveMonsters.IsValidIndex(i))
		{
			ATerminidBase* Monster = ActiveMonsters[i];
			if (IsValid(Monster) && Monster->IsValidLowLevel())
			{
				ETerminidType Type = Monster->GetTerminidType();
				int32* CountPtr = ActiveMonsterCounts.Find(Type);
				if (CountPtr)
				{
					(*CountPtr)++;
				}
				else
				{
					ActiveMonsterCounts.Add(Type, 1);
				}
			}
		}
	}
}

void ATerminidSpawner::CleanupInvalidMonsters()
{
	// 무효하거나 죽은 몬스터들 안전하게 제거
	for (int32 i = ActiveMonsters.Num() - 1; i >= 0; --i)
	{
		if (ActiveMonsters.IsValidIndex(i))
		{
			ATerminidBase* Monster = ActiveMonsters[i];
			if (!IsValid(Monster) || !Monster->IsValidLowLevel() || !Monster->IsAlive())
			{
				ActiveMonsters.RemoveAtSwap(i);
			}
		}
	}

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

// TakeDamage 오버라이드 - 폭발 데미지만 허용 (간단한 방식)
float ATerminidSpawner::TakeDamage(float DamageAmount, const struct FDamageEvent& DamageEvent,
                                   class AController* EventInstigator, class AActor* DamageCauser)
{
	// 이미 파괴된 경우 데미지 무시
	if (bIsDestroyed)
	{
		return 0.0f;
	}

	// DamageCauser 이름으로 폭발물 확인 (간단한 방식)
	FString DamageCauserName = DamageCauser ? DamageCauser->GetName() : TEXT("Unknown");

	bool bIsExplosiveDamage = DamageCauserName.Contains(TEXT("Grenade")) ||
		DamageCauserName.Contains(TEXT("Explosive")) ||
		DamageCauserName.Contains(TEXT("Rocket")) ||
		DamageCauserName.Contains(TEXT("Stratagem")) ||
		DamageCauserName.Contains(TEXT("Bomb"));

	if (bIsExplosiveDamage)
	{
		// 부모 TakeDamage 호출
		float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

		// 폭발 이펙트
		OnExplosionHit(GetActorLocation(), DamageAmount);

		return ActualDamage;
	}
	// 비주얼 이펙트만
	OnExplosionHit(GetActorLocation(), 0.0f);

	return 0.0f; // 데미지 무시
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

	// 스폰 중지
	StopSpawning();

	// 모든 몬스터 정리
	//ClearAllMonsters();

	// 스포너 메시를 파괴된 메시로 전환
	SwitchToDestroyedMesh();

	// 블로킹 메시 생성 (선택적)
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
}

void ATerminidSpawner::SwitchToDestroyedMesh()
{
	if (!NormalMeshComponent || !DestroyedMeshComponent)
	{
		return;
	}

	// 기본 메시를 숨기고 콜리전 비활성화
	NormalMeshComponent->SetVisibility(false);
	NormalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 파괴된 메시를 보이게 하고 콜리전 활성화
	DestroyedMeshComponent->SetVisibility(true);
	DestroyedMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	// 파괴된 메시가 기본 메시와 같은 트랜스폼을 가지도록 설정
	DestroyedMeshComponent->SetRelativeTransform(FTransform::Identity);
}

// HealthComponent 이벤트 핸들러들
void ATerminidSpawner::OnHealthComponentDeath(AActor* Victim)
{
	DestroySpawner();
}

void ATerminidSpawner::OnDamaged(float Damage, AActor* DamageCauser, AController* EventInstigator,
                                 TSubclassOf<UDamageType> DamageType)
{
	// 이미 파괴된 경우 무시
	if (bIsDestroyed)
	{
		return;
	}

	// 현재 체력 업데이트 (HealthComponent와 동기화)
	if (Health)
	{
		CurrentHealth = Health->GetHealthPercent() * MaxHealth;
	}

	// 폭발 이팩트 이벤트 호출 (Blueprint에서 구현)
	FVector HitLocation = GetActorLocation();
	OnExplosionHit(HitLocation, Damage);

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
