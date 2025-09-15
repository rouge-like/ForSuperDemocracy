#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "TerminidAIController.generated.h"

UCLASS()
class FORSUPERDEMOCRACY_API ATerminidAIController : public AAIController
{
    GENERATED_BODY()

public:
    ATerminidAIController();

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

public:
    // AI 행동 제어
    UFUNCTION(BlueprintCallable, Category = "AI")
    void StartAI();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void StopAI();

private:
    // AI가 활성화되어 있는지 확인
    bool bAIActive;
};