#include "OSC/UI/CompassEntry.h"

#include "Components/TextBlock.h"


void UCompassEntry::SetDistanceText(const float Distance) const
{
	FString text = FString::FromInt(Distance / 100) + "m";
	DistanceText->SetText(FText::FromString(text));
}

void UCompassEntry::SetIndexText(const int32 Index) const
{
	FString text = "[" + FString::FromInt(Index + 1) + "]";
	IndexText->SetText(FText::FromString(text));
}
