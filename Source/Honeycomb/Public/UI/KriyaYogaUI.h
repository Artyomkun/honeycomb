#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "Components/WrapBox.h"
#include "KriyaYogaUI.generated.h"

class UKriyaYogaSystem;
class AGameManager;

UCLASS(BlueprintType, Blueprintable)
class HONEYCOMB_API UKriyaYogaUI: public UUserWidget
{
    GENERATED_BODY()

    public:
        UKriyaYogaUI(const FObjectInitializer& ObjectInitializer);

        UFUNCTION(BlueprintCallable, Category = "UI|Kriya")
        void OnKriyaTechniqueClicked(int32 KriyaLevel);

        UFUNCTION(BlueprintCallable, Category = "UI|Kriya")
        void RefreshUI();

        UFUNCTION(BlueprintCallable, Category = "UI|Kriya")
        void InitializeUI();

    protected:
        virtual void NativeConstruct() override;
        virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
        virtual void NativeDestruct() override;

        UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI|Kriya")
        UPanelWidget* KriyaContainer;

        UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI|Chakra")
        UPanelWidget* ChakraStatusContainer;

        UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI|Energy")
        UProgressBar* KundaliniSlider;

        UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI|Energy")
        UTextBlock* KundaliniText;

        UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI|Energy")
        UImage* KundaliniFill;

        UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI|Energy")
        UProgressBar* PranaSlider;

        UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI|Energy")
        UTextBlock* PranaText;

        UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI|Energy")
        UImage* PranaFill;

        UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI|Controls")
        UButton* StartPracticeButton;

        UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI|Controls")
        UButton* StopPracticeButton;


        UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI|Controls")
        UTextBlock* PracticeStatusText;

        UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI|Controls")
        UProgressBar* PracticeProgressSlider;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Classes")
        TSubclassOf<UUserWidget> KriyaTechniqueWidgetClass;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Classes")
        TSubclassOf<UUserWidget> ChakraStatusWidgetClass;
        
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Energy")
        float MaxEnergy = 1000.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Energy")
        FLinearColor KundaliniLowColor = FLinearColor::Red;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Energy")
        FLinearColor KundaliniHighColor = FLinearColor::Yellow;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Energy")
        FLinearColor PranaLowColor = FLinearColor::Blue;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Energy")
        FLinearColor PranaHighColor = FLinearColor(0.0f, 1.0f, 1.0f);
        
        static constexpr int32 NUM_KRIYA_LEVELS = 12;
        static constexpr int32 NUM_CHAKRAS = 12;

    private:
        void CreateKriyaTechniqueDisplays();
        void CreateChakraStatusDisplays();
        void SetupButtons();
        void UpdateEnergyDisplays();
        void UpdatePracticeStatus();
        void UpdateChakraStatus();

        FString GetKriyaName(int32 Level) const;
        FString GetKriyaDescription(int32 Level) const;
        FLinearColor GetChakraColor(int32 Level) const;

        void UpdateFillColor(UImage* Fill, float Percent, const FLinearColor& LowColor, const FLinearColor& HighColor);
        
        UFUNCTION()
        void OnStartPracticeClicked();

        UFUNCTION()
        void OnStopPracticeClicked();

        UPROPERTY()
        UKriyaYogaSystem* KriyaSystem;

        UPROPERTY()
        AGameManager* GameManager;

        UPROPERTY()
        TArray<UUserWidget*> KriyaDisplays;

        UPROPERTY()
        TArray<UUserWidget*> ChakraStatusDisplays;

        bool bIsInitialized;

        FTimerHandle UpdateTimerHandle;

    public:
        
        DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKriyaTechniqueSelected, int32, KriyaLevel);
        DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnKriyaPracticeStarted);
        DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnKriyaPracticeStopped);

        UPROPERTY(BlueprintAssignable, Category = "UI|Events")
        FOnKriyaTechniqueSelected OnKriyaTechniqueSelected;

        UPROPERTY(BlueprintAssignable, Category = "UI|Events")
        FOnKriyaPracticeStarted OnKriyaPracticeStarted;

        UPROPERTY(BlueprintAssignable, Category = "UI|Events")
        FOnKriyaPracticeStopped OnKriyaPracticeStopped;
};
