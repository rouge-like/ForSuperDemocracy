#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TerminidTypes.h"
#include "TerminidFSM.generated.h"

class ATerminidBase;

// FSM 상태 함수 포인터 타입 정의
DECLARE_DELEGATE_OneParam(FTerminidStateDelegate, float);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FORSUPERDEMOCRACY_API UTerminidFSM : public UActorComponent
{
    GENERATED_BODY()

public:    
    UTerminidFSM();

protected:
    virtual void BeginPlay() override;

public:    
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // 현재 상태
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FSM")
    ETerminidState CurrentState;

    // 이전 상태 (디버깅 및 상태 복원용)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FSM")
    ETerminidState PreviousState;

    // 현재 상태 진입 시간
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FSM")
    float CurrentStateTime;

    // 상태 변경 가능 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FSM")
    bool bCanChangeState;

    // 메인 FSM 업데이트 함수
    UFUNCTION(BlueprintCallable, Category = "FSM")
    void UpdateStateMachine(float DeltaTime);

    // 상태 변경 함수
    UFUNCTION(BlueprintCallable, Category = "FSM")
    void ChangeState(ETerminidState NewState);

    // 상태 강제 변경 (제한 무시)
    UFUNCTION(BlueprintCallable, Category = "FSM")
    void ForceChangeState(ETerminidState NewState);

    // 이전 상태로 복원
    UFUNCTION(BlueprintCallable, Category = "FSM")
    void RevertToPreviousState();

    // 현재 상태 확인 함수들
    UFUNCTION(BlueprintPure, Category = "FSM")
    FORCEINLINE ETerminidState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "FSM")
    FORCEINLINE ETerminidState GetPreviousState() const { return PreviousState; }

    UFUNCTION(BlueprintPure, Category = "FSM")
    FORCEINLINE float GetCurrentStateTime() const { return CurrentStateTime; }

    UFUNCTION(BlueprintPure, Category = "FSM")
    FORCEINLINE bool CanChangeState() const { return bCanChangeState; }

    // 특정 상태인지 확인
    UFUNCTION(BlueprintPure, Category = "FSM")
    FORCEINLINE bool IsInState(ETerminidState State) const { return CurrentState == State; }

    // 상태 그룹 확인 함수들
    UFUNCTION(BlueprintPure, Category = "FSM")
    bool IsInCombatState() const;

    UFUNCTION(BlueprintPure, Category = "FSM")
    bool IsInMovementState() const;

    UFUNCTION(BlueprintPure, Category = "FSM")
    bool IsInPassiveState() const;

    // 상태 변경 제한 설정
    UFUNCTION(BlueprintCallable, Category = "FSM")
    void SetStateChangeLock(bool bLocked);

    // 일정 시간 후 상태 변경
    UFUNCTION(BlueprintCallable, Category = "FSM")
    void ChangeStateAfterTime(ETerminidState NewState, float Delay);

    // Hurt 상태 특수 처리 (일정 시간 후 자동 복원)
    UFUNCTION(BlueprintCallable, Category = "FSM")
    void EnterHurtState(float HurtDuration = 0.5f);

private:
    // Terminid 소유자 참조
    UPROPERTY()
    ATerminidBase* OwnerTerminid;

    // 상태별 처리 함수들
    void ProcessIdleState(float DeltaTime);
    void ProcessPatrolState(float DeltaTime);
    void ProcessChaseState(float DeltaTime);
    void ProcessAttackState(float DeltaTime);
    void ProcessHurtState(float DeltaTime);
    void ProcessDeathState(float DeltaTime);
    void ProcessSwarmState(float DeltaTime);
    void ProcessFleeState(float DeltaTime);

    // 상태 진입/종료 함수들
    void OnEnterState(ETerminidState NewState);
    void OnExitState(ETerminidState OldState);

    // 상태별 진입 함수들
    void OnEnterIdleState();
    void OnEnterPatrolState();
    void OnEnterChaseState();
    void OnEnterAttackState();
    void OnEnterHurtState();
    void OnEnterDeathState();
    void OnEnterSwarmState();
    void OnEnterFleeState();

    // 상태별 종료 함수들
    void OnExitIdleState();
    void OnExitPatrolState();
    void OnExitChaseState();
    void OnExitAttackState();
    void OnExitHurtState();
    void OnExitDeathState();
    void OnExitSwarmState();
    void OnExitFleeState();

    // 상태 변경 유효성 검사
    bool CanTransitionTo(ETerminidState NewState) const;

    // 타이머 핸들들
    FTimerHandle StateChangeTimerHandle;
    FTimerHandle HurtRecoveryTimerHandle;

    // 내부 플래그들
    bool bIsInitialized;
    bool bStateChangeLocked;
};