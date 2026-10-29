#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "UIWorldDisplay.generated.h"

UCLASS()
class HONEYCOMB_API UIWorldDisplay: public UUserWidget
{
    GENERATED_BODY()

    public:
        UFUNCTION(BlueprintCallable, Category = "UI")
        void UpdateUI(int32 CurrentWorldIndex);

    protected:
        UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "UI Elements")
        UTextBlock* WorldText;

        UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "UI Elements")
        UProgressBar* WorldProgress;

        UPROPERTY(BlueprintReadWrite, meta = (BindWidget), Category = "UI Elements")
        UImage* WorldIcon;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Settings")
        FLinearColor StartColor = FLinearColor::Red;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Settings")
        FLinearColor EndColor = FLinearColor::Green;

    private:
        FString FormatWorldText(int32 CurrentWorldIndex) const;
};