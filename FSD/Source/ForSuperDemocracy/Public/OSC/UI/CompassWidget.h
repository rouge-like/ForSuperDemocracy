#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CompassWidget.generated.h"

class UCompassEntry;
class UImage;
class UPanelWidget;
class UMissionComponent;

UCLASS()
class FORSUPERDEMOCRACY_API UCompassWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeOnInitialized() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Compass")
    void InitializeCompass(UMissionComponent* InMissionComponent);

protected:
    void UpdateTargets(const float PlayerYaw);
    void UpdateHeadingTape(const float PlayerYaw);
    void HideAllEntries();
    UCompassEntry* GetOrCreateEntry(int32 Index);

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Compass")
    TObjectPtr<UPanelWidget> EntryContainer = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Compass")
    TSubclassOf<UCompassEntry> EntryWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "Compass", meta = (ClampMin = "1"))
    int32 MaxVisibleTargets = 5;

    UPROPERTY(EditDefaultsOnly, Category = "Compass")
    float OffsetY = -20.f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Compass")
    float Size = 50.f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Compass")
    float PixelsPerDegree = 4.f;

    UPROPERTY(EditDefaultsOnly, Category = "Compass")
    float FadeAngleThreshold = 120.f;

    UPROPERTY(Transient)
    TWeakObjectPtr<UMissionComponent> MissionComponent;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UCompassEntry>> EntryPool;

    UPROPERTY(EditDefaultsOnly, meta = (BindWidget), Category = "Compass")
    TObjectPtr<UImage> HeadingTape;

    UPROPERTY(EditDefaultsOnly)
    UMaterial* HeadingMaterial;
    UPROPERTY()
    UMaterialInstanceDynamic* HeadingTapeMaterialInstance;

    float DegreePerPixel = 360.f / 1279.f;
};
