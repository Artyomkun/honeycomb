#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HoneycombAbilityIndicator.generated.h"

/** Code-only ability row. AVRUIManager sets the "AbilityName" text. */
UCLASS()
class HONEYCOMB_API UHoneycombAbilityIndicator : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeOnInitialized() override;
};
