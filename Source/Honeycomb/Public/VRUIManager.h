#pragma once

#include "HoneycombCore.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Systems/AtlantisLoreSystem.h"
#include "VRUIManager.generated.h"

class UWidgetComponent;
class UUserWidget;
class UProgressBar;
class UImage;
class UTextBlock;
class UCanvasPanel;
class UHorizontalBox;
class UVerticalBox;
class UOverlay;

UENUM(BlueprintType)
enum class EMoodIndicator: uint8
{
    Neutral UMETA(DisplayName = "Neutral"),
    Happy UMETA(DisplayName = "Happy"),
    Sad UMETA(DisplayName = "Sad"),
    Angry UMETA(DisplayName = "Angry"),
    Calm UMETA(DisplayName = "Calm"),
    Love UMETA(DisplayName = "Love")
};

USTRUCT(BlueprintType)
struct FChakraStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Value = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor Color = FLinearColor::White;
};

UCLASS(Blueprintable, BlueprintType)
class HONEYCOMB_API AVRUIManager: public AActor
{
    GENERATED_BODY()

    public:
        AVRUIManager();

        static AVRUIManager* GetInstance(UWorld* World);

    protected:
        virtual void BeginPlay() override;
        virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    public:
        UFUNCTION(BlueprintCallable, Category = "VR UI")
        void UpdateHealthBar(float Percentage);

        UFUNCTION(BlueprintCallable, Category = "VR UI")
        void UpdateEnergyBar(float Percentage);

        UFUNCTION(BlueprintCallable, Category = "VR UI")
        void SetEnergyBarVisible(bool bVisible);

        UFUNCTION(BlueprintCallable, Category = "VR UI")
        void ShowMeditationProgress(float Progress);

        UFUNCTION(BlueprintCallable, Category = "VR UI")
        void HideMeditationProgress();

        UFUNCTION(BlueprintCallable, Category = "VR UI")
        void ShowMeditationVFX();

        UFUNCTION(BlueprintCallable, Category = "VR UI")
        void HideMeditationVFX();

        UFUNCTION(BlueprintCallable, Category = "VR UI")
        void ShowNotification(const FString& Message);

        UFUNCTION(BlueprintCallable, Category = "VR UI")
        void UpdateChakraStatus(const TArray<FChakraStatus>& Status);

        UFUNCTION(BlueprintCallable, Category = "VR UI")
        void AddChakraIndicator(const FString& Name, FLinearColor Color);

        UFUNCTION(BlueprintCallable, Category = "VR UI")
        void UpdateKarmaIndicator(int32 Index);

        UFUNCTION(BlueprintCallable, Category = "VR UI")
        void UpdateMoodIndicator(EMoodIndicator Mood);

        UFUNCTION(BlueprintCallable, Category = "VR UI")
        void UpdateAbilitiesList(const TArray<FString>& Abilities);

        UFUNCTION(BlueprintCallable, Category = "VR UI")
        void AddAbility(const FString& AbilityName);

        UFUNCTION(BlueprintCallable, Category = "VR UI")
        void RemoveAbility(const FString& AbilityName);

        UFUNCTION(BlueprintCallable, Category = "VR UI")
        void UpdateQuestInfo(const FString& Title, float Progress);

        UFUNCTION(BlueprintCallable, Category = "VR UI")
        void ShowUI(bool bShow);

        UFUNCTION(BlueprintCallable, Category = "VR UI")
        void ToggleUI();

        UFUNCTION(BlueprintCallable, Category = "VR UI")
        void ShowMemoryFlash(const FAtlantisMemory& Memory);

    protected:
        UPROPERTY()
        UUserWidget* MainWidget;

        UPROPERTY()
        UProgressBar* HealthBar;

        UPROPERTY()
        UProgressBar* EnergyBar;

        UPROPERTY()
        UOverlay* EnergyContainer;

        UPROPERTY()
        UCanvasPanel* MeditationPanel;

        UPROPERTY()
        UProgressBar* MeditationProgress;

        UPROPERTY()
        UTextBlock* MeditationText;

        UPROPERTY()
        UTextBlock* NotificationText;

        UPROPERTY()
        UHorizontalBox* ChakraContainer;

        UPROPERTY()
        TSubclassOf<UUserWidget> ChakraIndicatorClass;

        UPROPERTY()
        TMap<FString, UUserWidget*> ChakraIndicators;

        UPROPERTY()
        UImage* KarmaImage;

        UPROPERTY()
        UImage* MoodImage;

        UPROPERTY()
        TArray<UTexture2D*> KarmaTextures;

        UPROPERTY()
        TArray<UTexture2D*> MoodTextures;

        UPROPERTY()
        UVerticalBox* AbilitiesContainer;

        UPROPERTY()
        TSubclassOf<UUserWidget> AbilityIndicatorClass;

        UPROPERTY()
        TMap<FString, UUserWidget*> AbilityIndicators;

        UPROPERTY()
        UTextBlock* QuestTitle;

        UPROPERTY()
        UProgressBar* QuestProgress;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR UI")
        TSubclassOf<UUserWidget> WidgetClass;

        void CreateUI();
        void InitializeWidgetReferences();
        void UpdateUIVisibility();

        static AVRUIManager* Instance;

    private:
        bool bIsUIVisible = true;
};
