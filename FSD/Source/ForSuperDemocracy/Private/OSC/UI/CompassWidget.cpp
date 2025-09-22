#include "OSC/UI/CompassWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "OSC/MissionComponent.h"
#include "OSC/UI/CompassEntry.h"

void UCompassWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (!EntryContainer)
    {
        return;
    }

    if (!EntryWidgetClass)
    {
        EntryWidgetClass = UImage::StaticClass();
    }
    
    EntryPool.Reserve(MaxVisibleTargets);
    for (int32 Index = 0; Index < MaxVisibleTargets; ++Index)
    {
        if (UCompassEntry* Entry = GetOrCreateEntry(Index))
        {
            Entry->SetVisibility(ESlateVisibility::Hidden);
        }
    }
    HeadingTapeMaterialInstance = UMaterialInstanceDynamic::Create(HeadingMaterial, this);
    HeadingTape->SetBrushFromMaterial(HeadingTapeMaterialInstance);
}

void UCompassWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    
    APawn* OwnerPawn = GetOwningPlayerPawn();
    if (!OwnerPawn) return;
    
    const APlayerController* OwnerPC = GetOwningPlayer();
    if (!OwnerPC) return;
    
    const FRotator ViewRot = OwnerPC ? OwnerPC->GetControlRotation() : OwnerPawn->GetActorRotation();
    const float PlayerYaw = ViewRot.Yaw;
    
    UpdateTargets(PlayerYaw);
    UpdateHeadingTape(PlayerYaw);
}

void UCompassWidget::InitializeCompass(UMissionComponent* InMissionComponent)
{
    MissionComponent = InMissionComponent;
    UpdateTargets(0);
}

void UCompassWidget::UpdateTargets(const float PlayerYaw)
{
    if (!EntryContainer)
    {
        return;
    }

    APawn* OwnerPawn = GetOwningPlayerPawn();

    if (!MissionComponent.IsValid() || !OwnerPawn)
    {
        HideAllEntries();
        return;
    }

    const FVector PawnLocation = OwnerPawn->GetActorLocation();

    TArray<FVector> Locations = MissionComponent->GetActiveObjectiveLocations();
    if (Locations.Num() == 0)
    {
        HideAllEntries();
        return;
    }

    Locations.Sort([&PawnLocation](const FVector& LHS, const FVector& RHS)
    {
        return FVector::DistSquared(LHS, PawnLocation) < FVector::DistSquared(RHS, PawnLocation);
    });

    const int32 VisibleCount = FMath::Min(MaxVisibleTargets, Locations.Num());

    for (int32 Index = 0; Index < VisibleCount; ++Index)
    {
        UCompassEntry* Entry = GetOrCreateEntry(Index);
        if (!Entry)
        {
            continue;
        }

        const FVector& TargetLocation = Locations[Index];
        FVector ToTarget = TargetLocation - PawnLocation;
        float Distance = ToTarget.Size();
        ToTarget.Z = 0.f;

        if (ToTarget.IsNearlyZero())
        {
            Entry->SetVisibility(ESlateVisibility::Hidden);
            continue;
        }

        const float TargetYaw = ToTarget.Rotation().Yaw;
        const float DeltaYaw = FMath::FindDeltaAngleDegrees(PlayerYaw, TargetYaw);

        Entry->SetVisibility(ESlateVisibility::HitTestInvisible);
        Entry->SetRenderTranslation(FVector2D(DeltaYaw * PixelsPerDegree, 0));
        Entry->SetDistanceText(Distance);
        float Abs = FMath::Abs(DeltaYaw);
        const bool bCentered = Abs > FadeAngleThreshold;
        Entry->SetRenderOpacity(bCentered ? 0 : 1.f);
    }

    for (int32 Index = VisibleCount; Index < EntryPool.Num(); ++Index)
    {
        if (UCompassEntry* Entry = EntryPool[Index])
        {
            Entry->SetVisibility(ESlateVisibility::Hidden);
        }
    }
}

void UCompassWidget::UpdateHeadingTape(const float PlayerYaw)
{
    float Normalized = PlayerYaw / 360.f;
    HeadingTapeMaterialInstance->SetScalarParameterValue("Yaw", -Normalized);
    // HeadingTape->SetRenderTranslation(FVector2D(PlayerYaw / PixelsPerDegree, 0.f));
}

void UCompassWidget::HideAllEntries()
{
    for (UCompassEntry* Entry : EntryPool)
    {
        if (Entry)
        {
            Entry->SetVisibility(ESlateVisibility::Hidden);
        }
    }
}

UCompassEntry* UCompassWidget::GetOrCreateEntry(int32 Index)
{
    if (EntryPool.IsValidIndex(Index) && EntryPool[Index])
    {
        return EntryPool[Index];
    }

    if (!EntryContainer || !WidgetTree)
    {
        return nullptr;
    }

    if (!EntryWidgetClass)
    {
        EntryWidgetClass = UImage::StaticClass();
    }

    UCompassEntry* NewEntry = WidgetTree->ConstructWidget<UCompassEntry>(EntryWidgetClass);
    if (!NewEntry)
    {
        return nullptr;
    }

    NewEntry->SetVisibility(ESlateVisibility::Hidden);
    NewEntry->SetIndexText(Index);
    
    UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(EntryContainer->AddChild(NewEntry));                                                                                                                                        
    if (CanvasSlot)                                                                                                                                                                                                                   
    {                                                                                                                                                                                                                                 
        FAnchors Anchors(0.5f, 1.f);                                                                                                                                                                                                 
        CanvasSlot->SetAnchors(Anchors); // 앵커를 하단 중앙으로                                                                                                                                                             
        CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f)); // 정렬을 위젯 중앙으로                                                                                                                                                      
        CanvasSlot->SetPosition(FVector2D(0, OffsetY));
        CanvasSlot->SetSize(FVector2D(Size, Size));
    }
    

    if (EntryPool.Num() <= Index)
    {
        EntryPool.SetNum(Index + 1);
    }

    EntryPool[Index] = NewEntry;
    return NewEntry;
}
