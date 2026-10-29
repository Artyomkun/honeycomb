#pragma once

#include "HoneycombCore.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InputSystem.generated.h"

UCLASS()
class HONEYCOMBEDITOR_API UInputSystem: public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

    public:
        UFUNCTION(BlueprintCallable, Category = "Editor|Input")
        static void SetInputHandlingBoth();

        UFUNCTION(BlueprintCallable, Category = "Editor|Input")
        static void SetInputHandlingLegacy();

        UFUNCTION(BlueprintCallable, Category = "Editor|Input")
        static void SetInputHandlingEnhanced();

        UFUNCTION(BlueprintCallable, Category = "Editor|Input")
        static FString GetCurrentInputHandling();

    private:
        static void SetInputHandlingMode(const FString& Mode);
        static void UpdateProjectSettings(const FString& SettingName, const FString& Value);
        static void SaveProjectSettings();
        static void ShowNotification(const FString& Message);
};
