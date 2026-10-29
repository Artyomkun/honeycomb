#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HoneycombChakraIndicator.generated.h"

/** Code-only chakra dot. AVRUIManager tints the "ChakraIcon" image. */
UCLASS()
class HONEYCOMB_API UHoneycombChakraIndicator : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeOnInitialized() override;
};
