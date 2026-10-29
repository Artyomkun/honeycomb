#pragma once

#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "Components/WrapBox.h"
#include "ChakraUI.generated.h"

class UChakraSystem;
class AGameManager;
class UUIChakraDisplay;

USTRUCT(BlueprintType)
struct FChakraInfoData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Mantra;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Abilities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor Color;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Level;

    FChakraInfoData(): Color(FLinearColor::White), Level(0) {}
};

UCLASS(BlueprintType, Blueprintable)
class HONEYCOMB_API UChakraUI : public UUserWidget
{
    GENERATED_BODY()

    public:
        UChakraUI(const FObjectInitializer& ObjectInitializer);

        UFUNCTION(BlueprintCallable, Category = "UI|Chakra")
        void UpdateUI(int32 CurrentLevel);

        UFUNCTION(BlueprintCallable, Category = "UI|Chakra")
        void UpdateChakraStates();

        UFUNCTION(BlueprintCallable, Category = "UI|Chakra")
        void OnChakraClicked(int32 ChakraLevel);

        UFUNCTION(BlueprintCallable, Category = "UI|Chakra")
        void InitializeUI();

        UFUNCTION(BlueprintCallable, Category = "UI|Chakra")
        void ShowChakraInfo(const FChakraInfoData& FChakraData);

    protected:
        virtual void NativeConstruct() override;
        virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
        virtual void NativeDestruct() override;

        UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI|Chakra")
        UPanelWidget* ChakraContainer;

        UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "UI|Info")
        UTextBlock* ChakraNameText;

        UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "UI|Info")
        UTextBlock* ChakraDescriptionText;

        UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "UI|Info")
        UTextBlock* ChakraMantraText;

        UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "UI|Info")
        UTextBlock* ChakraAbilitiesText;

        UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "UI|Info")
        UPanelWidget* ChakraInfoPanel;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Classes")
        TSubclassOf<UUserWidget> ChakraWidgetClass;
        
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Colors")
        TArray<FLinearColor> ChakraColors;
        
        static constexpr int32 NUM_CHAKRAS = 12;

    private:
        void CreateChakraDisplays();

        FString GetChakraName(int32 Level) const;
        FString GetChakraDescription(int32 Level) const;
        FString GetChakraMantra(int32 Level) const;
        TArray<FString> GetChakraAbilities(int32 Level) const;
        FLinearColor GetChakraColor(int32 Level) const;
        void InitializeDefaultColors();
        UPROPERTY()
        UChakraSystem* ChakraSystem;

        UPROPERTY()
        AGameManager* GameManager;

        UPROPERTY()
        TArray<UUIChakraDisplay*> ChakraDisplays;

        bool bIsInitialized;
        FTimerHandle UpdateTimerHandle;
        int32 SelectedChakraLevel;

    public:
        DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChakraSelected, int32, ChakraLevel);
        DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChakraInfoDisplayed, const FChakraInfoData&, FChakraData);

        UPROPERTY(BlueprintAssignable, Category = "UI|Events")
        FOnChakraSelected OnChakraSelected;

        UPROPERTY(BlueprintAssignable, Category = "UI|Events")
        FOnChakraInfoDisplayed OnChakraInfoDisplayed;
};
