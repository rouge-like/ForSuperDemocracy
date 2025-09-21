#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CompassEntry.generated.h"

class UImage;
class UTextBlock;

UCLASS()
class FORSUPERDEMOCRACY_API UCompassEntry : public UUserWidget
{
    GENERATED_BODY()
public:
    void SetDistanceText(const float Distance) const;
    void SetIndexText(const int32 Index) const;

protected:
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Compass")
    TObjectPtr<UTextBlock> DistanceText;
    
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Compass")
    TObjectPtr<UTextBlock> IndexText;
};
