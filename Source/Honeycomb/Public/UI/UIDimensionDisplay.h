#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
#include "Components/Image.h"
#include "Styling/SlateColor.h"
#include "UIDimensionDisplay.generated.h"

UCLASS(BlueprintType, Blueprintable)
class HONEYCOMB_API UUIDimensionDisplay: public UUserWidget
{
    GENERATED_BODY()

    public:
        UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "UI|Dimension")
        void UpdateUI(int32 Dimension);

        UFUNCTION(BlueprintCallable, Category = "UI|Dimension")
        void UpdateUIExtended(int32 Dimension, bool bShowLabel = true, const FString& CustomPrefix = TEXT("Измерение:"));

        UFUNCTION(BlueprintCallable, Category = "UI|Dimension|Animation")
        void UpdateUIAnimated(int32 Dimension, float Duration = 0.5f);

        UFUNCTION(BlueprintCallable, Category = "UI|Dimension")
        void SetDimensionName(const FString& Name);

        UFUNCTION(BlueprintCallable, Category = "UI|Dimension")
        void SetDimensionColor(const FLinearColor& Color);

        UFUNCTION(BlueprintCallable, Category = "UI|Dimension")
        void SetDimensionIndex(int32 Dimension);

        UFUNCTION(BlueprintCallable, Category = "UI|Dimension|Animation")
        void PlayTransitionAnimation(int32 NewDimension);

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Appearance")
        FSlateColor TextColor = FSlateColor(FLinearColor::White);

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Appearance", meta = (ClampMin = "8", ClampMax = "72"))
        int32 FontSize = 24;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Appearance")
        bool bIsBold = true;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Advanced")
        bool bUseRichText = false;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Advanced")
        bool bShowIcon = false;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Advanced", meta = (EditCondition = "bShowIcon"))
        TArray<UTexture2D*> DimensionIcons;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Advanced")
        TMap<int32, FTextBlockStyle> DimensionStyles;

    protected:
        
        virtual void NativeConstruct() override;
        virtual void NativeDestruct() override;
        virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

        UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI|Widgets")
        UTextBlock* Label;

        UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "UI|Widgets")
        URichTextBlock* RichLabel;

        UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "UI|Widgets")
        UImage* DimensionIcon;

        FString FormatDimensionText(int32 Dimension) const;
        FString FormatRichText(int32 Dimension) const;

        void UpdateDimensionIcon(int32 Dimension);
        void ApplyTextStyle(int32 Dimension);
        void UpdateTextColor(const FLinearColor& Color);
        void UpdateDimensionVisual(const FLinearColor& Color, float Scale, float Opacity, float Progress);
        FLinearColor GetDimensionColor(int32 Dimension) const;

        int32 CurrentDimension;
        int32 TargetDimension;
        float AnimationTime;
        float AnimationDuration = 0.5f;
        bool bIsAnimating;
        int32 PreviousDimension;
        FTimerHandle UpdateTimerHandle;

        DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDimensionChanged, int32, NewDimension);
        DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDimensionAnimationComplete, int32, NewDimension);

        UPROPERTY()
        FOnDimensionChanged OnDimensionChanged;

        UPROPERTY(BlueprintAssignable, Category = "UI|Events")
        FOnDimensionChanged OnDimensionChangedEvent;

        UPROPERTY(BlueprintAssignable, Category = "UI|Events")
        FOnDimensionAnimationComplete OnAnimationComplete;
};