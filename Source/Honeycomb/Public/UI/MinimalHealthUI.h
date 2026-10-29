#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/WidgetComponent.h"
#include "MinimalHealthUI.generated.h"

class AVRPlayerController;
class UDiseaseSystem;
class UMoodSystem;
class AGameManager;

UCLASS(BlueprintType, Blueprintable)
class HONEYCOMB_API UMinimalHealthUI: public UUserWidget
{
    GENERATED_BODY()

    public:
        UMinimalHealthUI(const FObjectInitializer& ObjectInitializer);

        UFUNCTION(BlueprintCallable, Category = "UI|Health")
        void OnTemperatureChanged(float NewTemperature);

        UFUNCTION(BlueprintCallable, Category = "UI|Health")
        void OnHealthChanged(float NewHealthPercentage);

        UFUNCTION(BlueprintCallable, Category = "UI|Health")
        void RefreshUI();

    protected:
        UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI|Health")
        UImage* HealthBar;

        UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI|Health")
        UTextBlock* HealthText;

        UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI|Temperature")
        UTextBlock* TemperatureText;

        UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI|Temperature")
        UImage* TemperatureIndicator;

        UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI|StatusEffects")
        UPanelWidget* StatusEffectsContainer;
        
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Colors")
        FLinearColor HealthyColor = FLinearColor::Green;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Colors")
        FLinearColor InjuredColor = FLinearColor::Yellow;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Colors")
        FLinearColor CriticalColor = FLinearColor::Red;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Colors")
        FLinearColor NormalTempColor = FLinearColor::White;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Colors")
        FLinearColor FeverTempColor = FLinearColor::Red;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Colors")
        FLinearColor ColdTempColor = FLinearColor::Blue;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|StatusEffects")
        TSubclassOf<UUserWidget> StatusEffectWidgetClass;
        
        virtual void NativeConstruct() override;
        virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
        virtual void NativeDestruct() override;

        void FindPlayerAndSystems();
        void UpdateHealthDisplay();
        void UpdateTemperatureDisplay();
        void UpdateStatusEffects();
        void AddStatusEffect(const FString& Name, float Severity);

        FLinearColor GetHealthColor(float HealthPercentage) const;
        FLinearColor GetTemperatureColor(float Temperature) const;

    private:
        UPROPERTY()
        AVRPlayerController* Player;

        UPROPERTY()
        UDiseaseSystem* DiseaseSystem;

        UPROPERTY()
        UMoodSystem* MoodSystem;

        UPROPERTY()
        AGameManager* GameManager;

        FVector2D OriginalHealthBarSize;

        float CurrentHealthPercentage;
        float CurrentTemperature;

        FTimerHandle UpdateTimerHandle;

        bool bSystemsFound;

        TArray<UUserWidget*> ActiveStatusEffects;

    public:
        
        DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChangedEvent, float, NewHealth);
        DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTemperatureChangedEvent, float, NewTemperature);

        UPROPERTY(BlueprintAssignable, Category = "UI|Events")
        FOnHealthChangedEvent OnHealthChangedEvent;

        UPROPERTY(BlueprintAssignable, Category = "UI|Events")
        FOnTemperatureChangedEvent OnTemperatureChangedEvent;
};
