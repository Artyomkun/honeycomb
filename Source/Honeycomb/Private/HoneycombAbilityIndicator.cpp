#include "HoneycombAbilityIndicator.h"

#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"

void UHoneycombAbilityIndicator::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (!WidgetTree)
    {
        return;
    }

    UTextBlock* Name = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("AbilityName"));
    Name->SetText(FText::FromString(TEXT("Ability")));
    Name->SetColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.95f, 1.0f, 1.0f)));

    WidgetTree->RootWidget = Name;
}
