#include "Huxley/TerminidAIController.h"
#include "Huxley/TerminidBase.h"

ATerminidAIController::ATerminidAIController()
{
    PrimaryActorTick.bCanEverTick = false; // 기본 FSM이므로 AI Controller는 틱 불필요
    
    bAIActive = false;
}

void ATerminidAIController::BeginPlay()
{
    Super::BeginPlay();
    
    // AI 시작
    StartAI();
}

void ATerminidAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    // Terminid 빙의 시 AI 시작
    if (InPawn)
    {
        StartAI();
    }
}

void ATerminidAIController::OnUnPossess()
{
    // AI 정지
    StopAI();
    
    Super::OnUnPossess();
}

void ATerminidAIController::StartAI()
{
    if (bAIActive)
        return;
        
    bAIActive = true;
    
    // 기본 FSM이므로 별도 설정 불필요
    // Terminid 자체의 Tick에서 플레이어 감지 및 FSM 처리
}

void ATerminidAIController::StopAI()
{
    if (!bAIActive)
        return;
        
    bAIActive = false;
}