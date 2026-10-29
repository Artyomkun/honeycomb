#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
#include "Components/Image.h"
#include "UIChakraDisplay.generated.h"

UCLASS(BlueprintType, Blueprintable)
class HONEYCOMB_API UUIChakraDisplay: public UUserWidget
{
    GENERATED_BODY()

    public:

        UUIChakraDisplay(const FObjectInitializer& ObjectInitializer);

        UFUNCTION(BlueprintCallable, Category = "UI|Chakra")
        void UpdateUI(int32 ChakraLevel, const FString& UnusedParam = TEXT(""));

        UFUNCTION(BlueprintCallable, Category = "UI|Chakra")
        void UpdateChakra(int32 ChakraLevel);

        UFUNCTION(BlueprintCallable, Category = "UI|Chakra")
        void UpdateChakraWithType(int32 ChakraLevel, const FString& ChakraType);

        UFUNCTION(BlueprintCallable, Category = "UI|Chakra")
        void InitializeChakra(int32 ChakraIndex, const FLinearColor& Color);

        UFUNCTION(BlueprintCallable, Category = "UI|Chakra")
        void UpdateState(bool bIsUnlocked, bool bIsCurrent);

        UFUNCTION(BlueprintCallable, Category = "UI|Chakra")
        void SetTextPrefix(const FString& NewPrefix);

        UFUNCTION(BlueprintCallable, Category = "UI|Chakra")
        void AddLevelColor(int32 ChakraLevel, FLinearColor Color);

        UFUNCTION(BlueprintCallable, Category = "UI|Chakra")
        void AddLevelIcon(int32 ChakraLevel, UTexture2D* Icon);

    protected:
        UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI|Widgets")
        UTextBlock* Label;

        UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "UI|Widgets")
        URichTextBlock* RichLabel;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Settings")
        FString TextPrefix = TEXT("Чакра: Уровень");

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Settings")
        bool bUseRichText = false;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Settings")
        TMap<int32, FLinearColor> LevelColors;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Settings")
        FLinearColor DefaultColor = FLinearColor::White;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Settings")
        TArray<FLinearColor> ChakraColors;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Settings")
        bool bShowChakraIcon = false;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Settings", meta = (EditCondition = "bShowChakraIcon"))
        TMap<int32, UTexture2D*> ChakraIcons;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Settings", meta = (ClampMin = "8", ClampMax = "72"))
        int32 FontSize = 24;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Settings")
        bool bIsBold = true;
        
        UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "UI|Widgets")
        UImage* ChakraIcon;

    private:
        FString FormatChakraText(int32 ChakraLevel) const;
        FString FormatRichChakraText(int32 ChakraLevel) const;

        void UpdateTextColor(int32 ChakraLevel);
        void UpdateChakraIcon(int32 ChakraLevel);
        void ApplyChakraStyle(int32 ChakraLevel);
        FLinearColor GetChakraColor(int32 ChakraLevel) const;
        void StartPulseAnimation();
        void StopPulseAnimation();

        int32 CurrentChakraLevel;
        int32 ChakraIndex = 0;

        FLinearColor DisplayColor = FLinearColor::White;

    public:
        DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChakraLevelChanged, int32, NewChakraLevel);

        UPROPERTY(BlueprintAssignable, Category = "UI|Events")
        FOnChakraLevelChanged OnChakraLevelChanged;
};