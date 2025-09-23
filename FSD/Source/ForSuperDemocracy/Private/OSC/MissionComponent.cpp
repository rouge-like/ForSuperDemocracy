// Fill out your copyright notice in the Description page of Project Settings.


#include "OSC/MissionComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UMissionComponent::UMissionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UMissionComponent::BeginPlay()
{
    Super::BeginPlay();

    // ...
    
}


// Called every frame
void UMissionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                      FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // ...
}

void UMissionComponent::StartMission()
{
    // 초기화 및 첫 목표 시작
    if (!MissionData)
    {
        // if(GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Green, FString::Printf(TEXT("Mission Data null")));
        return;
    }
    if (!MissionData || MissionData->Objectives.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("MissionComponent::StartMission - MissionData is empty"));
        return;
    }
    
    CountedActors.Empty();
    ActiveObjectiveActors.Reset();
    Curr = 0;
    RemainSec = 0;
    State = EMissionState::Idle;
    ObjectiveIndex = 0;

    StartObjective(ObjectiveIndex);
}

void UMissionComponent::StopMission()
{
    GetWorld()->GetTimerManager().ClearTimer(TimerHandle_1Hz);
    CountedActors.Empty();
    ActiveObjectiveActors.Reset();
    Curr = 0;
    RemainSec = 0;
    ObjectiveIndex = INDEX_NONE;
    State = EMissionState::Idle;
}

TArray<FVector> UMissionComponent::GetActiveObjectiveLocations() const
{
    TArray<FVector> Locations;

    if (State != EMissionState::Running || !MissionData || !MissionData->Objectives.IsValidIndex(ObjectiveIndex))
    {
        return Locations;
    }

    const FMissionObjective& Obj = GetCurrentObjective();
    if (Obj.Type == EMissionType::Destroy || Obj.Type == EMissionType::ReachArea)
    {
        for (const TWeakObjectPtr<AActor>& Target : ActiveObjectiveActors)
        {
            if (const AActor* Actor = Target.Get())
            {
                Locations.Add(Actor->GetActorLocation());
            }
        }
    }

    return Locations;
}

void UMissionComponent::StartObjective(int32 Index)
{
    if (!MissionData || !MissionData->Objectives.IsValidIndex(Index))
    {
        UE_LOG(LogTemp, Warning, TEXT("MissionComponent::StartObjective - Invalid index"));
        return;
    }
    
    GetWorld()->GetTimerManager().ClearTimer(TimerHandle_1Hz);
    CountedActors.Empty();
    ActiveObjectiveActors.Reset();

    ObjectiveIndex = Index;
    Curr = 0;
    RemainSec = 0;
    State = EMissionState::Running;

    const FMissionObjective& Obj = GetCurrentObjective();

    // 타이머 설정: Survive는 Target(초) 우선, 그 외 TimeLimit 존재 시 제한시간 적용
    int32 InitialTime = 0;
    if (Obj.Type == EMissionType::Survive)
    {
        InitialTime = Obj.TimeLimit;
    }

    RemainSec = InitialTime;
    if (InitialTime > 0)
    {
        GetWorld()->GetTimerManager().SetTimer(TimerHandle_1Hz, this, &UMissionComponent::TickTimer, 1.0f, true);
        OnTimerTick.Broadcast(RemainSec);
    }

    OnObjectiveChanged.Broadcast(Obj);
    CacheObjectiveTargets(Obj);
    OnObjectiveUpdated.Broadcast(Curr, Obj.Target);
    
    //if(GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Green, FString::Printf(TEXT("Mission Start! %d , %d, %d"), Obj.Type, Obj.Target, InitialTime));
}

void UMissionComponent::CompleteObjective()
{
    GetWorld()->GetTimerManager().ClearTimer(TimerHandle_1Hz);
    ActiveObjectiveActors.Reset();
    OnObjectiveCompleted.Broadcast();

    const int32 NextIndex = ObjectiveIndex + 1;
    if (MissionData && MissionData->Objectives.IsValidIndex(NextIndex))
    {
        StartObjective(NextIndex);
    }
    else
    {
        State = EMissionState::Completed;
        OnMissionCompleted.Broadcast();
    }
}

void UMissionComponent::FailObjective()
{
    GetWorld()->GetTimerManager().ClearTimer(TimerHandle_1Hz);
    ActiveObjectiveActors.Reset();
    State = EMissionState::Failed;
    OnMissionFailed.Broadcast();
}

void UMissionComponent::TickTimer()
{
    if (RemainSec > 0)
    {
        --RemainSec;
        OnTimerTick.Broadcast(RemainSec);
        if (RemainSec <= 0)
        {
            if (MissionData && MissionData->Objectives.IsValidIndex(ObjectiveIndex))
            {
                const FMissionObjective& Obj = MissionData->Objectives[ObjectiveIndex];
                if (Obj.Type == EMissionType::Survive)
                {
                    CompleteObjective();
                }
                else
                {
                    FailObjective();
                }
            }
            else
            {
                FailObjective();
            }
        }
    }
}

void UMissionComponent::NotifyKill(FName EnemyTag)
{
    if (State != EMissionState::Running || !MissionData || !MissionData->Objectives.IsValidIndex(ObjectiveIndex))
        return;

    const FMissionObjective& Obj = MissionData->Objectives[ObjectiveIndex];
    if (Obj.Type != EMissionType::Kill)
        return;

    if (Obj.FilterTag.IsNone() || Obj.FilterTag == EnemyTag)
    {
        Curr = FMath::Clamp(Curr + 1, 0, Obj.Target);
        OnObjectiveUpdated.Broadcast(Curr, Obj.Target);
        if (Obj.Target > 0 && Curr >= Obj.Target)
        {
            CompleteObjective();
        }
    }
}

void UMissionComponent::NotifyDestroyed(AActor* Actor)
{
    if (State != EMissionState::Running || !MissionData || !MissionData->Objectives.IsValidIndex(ObjectiveIndex))
        return;

    const FMissionObjective& Obj = MissionData->Objectives[ObjectiveIndex];
    if (Obj.Type != EMissionType::Destroy || !Actor)
        return;

    // 클래스 필터 체크
    if (Obj.TargetClass && !Actor->IsA(Obj.TargetClass))
        return;

    // 태그 필터 체크
    if (!Obj.FilterTag.IsNone() && !Actor->ActorHasTag(Obj.FilterTag))
        return;

    // 중복 가드
    if (CountedActors.Contains(Actor))
        return;

    CountedActors.Add(Actor);
    for (int32 Index = ActiveObjectiveActors.Num() - 1; Index >= 0; --Index)
    {
        const TWeakObjectPtr<AActor>& Target = ActiveObjectiveActors[Index];
        if (!Target.IsValid() || Target.Get() == Actor)
        {
            ActiveObjectiveActors.RemoveAtSwap(Index);
        }
    }
    Curr = FMath::Clamp(Curr + 1, 0, Obj.Target);
    OnObjectiveUpdated.Broadcast(Curr, Obj.Target);
    if (Obj.Target > 0 && Curr >= Obj.Target)
    {
        CompleteObjective();
    }
    //if(GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Green, FString::Printf(TEXT("Destroy! %s , %d / %d"), *Obj.TargetClass->GetName(), Obj.Target, Curr));

}

void UMissionComponent::NotifyAreaEntered(FName Area)
{
    if (State != EMissionState::Running || !MissionData || !MissionData->Objectives.IsValidIndex(ObjectiveIndex))
        return;

    const FMissionObjective& Obj = MissionData->Objectives[ObjectiveIndex];
    if (Obj.Type != EMissionType::ReachArea)
        return;

    if (Obj.AreaTag.IsNone() || Obj.AreaTag == Area)
    {
        // ReachArea는 즉시 완료
        CompleteObjective();
    }
}

 

void UMissionComponent::CacheObjectiveTargets(const FMissionObjective& Objective)
{
    ActiveObjectiveActors.Reset();

    if (!GetWorld())
    {
        return;
    }

    if (Objective.Type == EMissionType::Destroy)
    {
        TArray<AActor*> Candidates;
        if (Objective.TargetClass)
        {
            UGameplayStatics::GetAllActorsOfClass(this, Objective.TargetClass, Candidates);
        }
        else if (!Objective.FilterTag.IsNone())
        {
            UGameplayStatics::GetAllActorsWithTag(this, Objective.FilterTag, Candidates);
        }

        for (AActor* Candidate : Candidates)
        {
            if (!IsValid(Candidate))
            {
                continue;
            }

            if (!Objective.FilterTag.IsNone() && !Candidate->ActorHasTag(Objective.FilterTag))
            {
                continue;
            }

            ActiveObjectiveActors.Add(Candidate);
        }
    }
    else if (Objective.Type == EMissionType::ReachArea)
    {
        if (!Objective.AreaTag.IsNone())
        {
            TArray<AActor*> Candidates;
            UGameplayStatics::GetAllActorsWithTag(this, Objective.AreaTag, Candidates);
            for (AActor* Candidate : Candidates)
            {
                if (IsValid(Candidate))
                {
                    ActiveObjectiveActors.Add(Candidate);
                }
            }
        }
    }
}



