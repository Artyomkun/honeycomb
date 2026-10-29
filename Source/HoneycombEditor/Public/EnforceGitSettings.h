#pragma once

#include "HoneycombCore.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EnforceGitSettings.generated.h"

UCLASS()
class HONEYCOMBEDITOR_API UEnforceGitSettings: public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

    public:
        UFUNCTION(BlueprintCallable, Category = "Editor|Git")
        static void EnforceGitSettings();

        UFUNCTION(BlueprintCallable, Category = "Editor|Git")
        static void SetTextAssetSerialization();

        UFUNCTION(BlueprintCallable, Category = "Editor|Git")
        static void SetVisibleMetaFiles();

        UFUNCTION(BlueprintCallable, Category = "Editor|Git")
        static bool AreGitSettingsEnforced();

        UFUNCTION(BlueprintCallable, Category = "Editor|Git")
        static FString GetCurrentAssetSerializationMode();

    private:
        static void UpdateProjectSettings(const FString& Section, const FString& Key, const FString& Value);
        static void SaveProjectSettings();
        static void ShowNotification(const FString& Message);
        static void LogMessage(const FString& Message);
};
