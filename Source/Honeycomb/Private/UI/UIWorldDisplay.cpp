#include "UIWorldDisplay.h"
#include "Kismet/GameplayStatics.h"

void UIWorldDisplay::UpdateUI(int32 CurrentWorldIndex)
{
    if (WorldText)
    {
        FString WorldTextString = FormatWorldText(CurrentWorldIndex);
        WorldText->SetText(FText::FromString(WorldTextString));
    }

    if (WorldProgress)
    {
        float ProgressValue = (CurrentWorldIndex + 1) / 54.0f;
        WorldProgress->SetPercent(ProgressValue);
    }

    if (WorldIcon)
    {
        FLinearColor NewColor = FMath::Lerp(StartColor, EndColor, FMath::Clamp((CurrentWorldIndex + 1) / 54.0f, 0.0f, 1.0f));
        WorldIcon->SetColorAndOpacity(NewColor);
    }
}

FString UIWorldDisplay::FormatWorldText(int32 CurrentWorldIndex) const
{
    return FString::Printf(TEXT("Мир: %d/54"), CurrentWorldIndex + 1);
}