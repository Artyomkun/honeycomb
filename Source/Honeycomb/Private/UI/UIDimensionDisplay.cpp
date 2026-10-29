#include "UIDimensionDisplay.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Animation/WidgetAnimation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Engine/Engine.h"

void UUIDimensionDisplay::NativeConstruct()
{
    Super::NativeConstruct();

    CurrentDimension = 0;
    TargetDimension = 0;
    AnimationTime = 0.0f;
    bIsAnimating = false;
    PreviousDimension = 0;

    if (!Label && !RichLabel)
    {
        UE_LOG(LogTemp, Warning, TEXT("UUIDimensionDisplay: Label or RichLabel is not bound!"));
    }

    UpdateUI(0);
}

void UUIDimensionDisplay::NativeDestruct()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(UpdateTimerHandle);
    }

    Super::NativeDestruct();
}

void UUIDimensionDisplay::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!bIsAnimating) return;

    AnimationTime += InDeltaTime;
    float Progress = FMath::Clamp(AnimationTime / AnimationDuration, 0.0f, 1.0f);
    float Eased = FMath::InterpEaseInOut(0.0f, 1.0f, Progress, 2.0f);

    int32 FromDim = PreviousDimension;
    int32 ToDim = TargetDimension;

    FLinearColor CurrentColor = FLinearColor::LerpUsingHSV(
        GetDimensionColor(FromDim),
        GetDimensionColor(ToDim),
        Eased
    );

    float CurrentScale = FMath::Lerp(0.8f, 1.2f, Eased);
    float CurrentOpacity = FMath::Lerp(1.0f, 0.6f, FMath::Sin(Progress * PI));

    UpdateDimensionVisual(CurrentColor, CurrentScale, CurrentOpacity, Eased);

    if (Progress >= 1.0f)
    {
        bIsAnimating = false;
        PreviousDimension = TargetDimension;
        OnAnimationComplete.Broadcast(TargetDimension);
    }
}

void UUIDimensionDisplay::UpdateUI(int32 Dimension)
{
    if (!Label && !RichLabel)
    {
        UE_LOG(LogTemp, Warning, TEXT("UUIDimensionDisplay: No text widget available!"));
        return;
    }

    PreviousDimension = CurrentDimension;
    CurrentDimension = Dimension;

    if (bUseRichText && RichLabel)
    {
        FString FormattedText = FormatRichText(Dimension);
        RichLabel->SetText(FText::FromString(FormattedText));
    }
    else if (Label)
    {
        FString FormattedText = FormatDimensionText(Dimension);
        Label->SetText(FText::FromString(FormattedText));
        ApplyTextStyle(Dimension);
    }

    if (bShowIcon && DimensionIcon)
    {
        UpdateDimensionIcon(Dimension);
    }

    OnDimensionChangedEvent.Broadcast(Dimension);

    UE_LOG(LogTemp, Verbose, TEXT("UUIDimensionDisplay: Updated to dimension %d"), Dimension);
}

void UUIDimensionDisplay::UpdateUIExtended(int32 Dimension, bool bShowLabel, const FString& CustomPrefix)
{
    if (!Label && !RichLabel)
    {
        return;
    }

    CurrentDimension = Dimension;

    FString TextToDisplay;
    if (bShowLabel)
    {
        TextToDisplay = FString::Printf(TEXT("%s %dD"), *CustomPrefix, Dimension);
    }
    else
    {
        TextToDisplay = FString::Printf(TEXT("%dD"), Dimension);
    }

    if (Label)
    {
        Label->SetText(FText::FromString(TextToDisplay));
    }
    else if (RichLabel)
    {
        RichLabel->SetText(FText::FromString(TextToDisplay));
    }
}

void UUIDimensionDisplay::UpdateUIAnimated(int32 Dimension, float Duration)
{
    if (Dimension == CurrentDimension)
    {
        return;
    }

    TargetDimension = Dimension;
    AnimationTime = 0.0f;
    bIsAnimating = true;

    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(UpdateTimerHandle);
        FTimerDelegate TimerDelegate;
        TimerDelegate.BindUObject(this, &UUIDimensionDisplay::UpdateUI, TargetDimension);
        GetWorld()->GetTimerManager().SetTimer(
            UpdateTimerHandle,
            TimerDelegate,
            Duration,
            false
        );
    }

    UpdateUI(Dimension);
}

FString UUIDimensionDisplay::FormatDimensionText(int32 Dimension) const
{
    return FString::Printf(TEXT("Измерение: %dD"), Dimension);
}

FString UUIDimensionDisplay::FormatRichText(int32 Dimension) const
{
    FString ColorHex = TextColor.GetSpecifiedColor().ToFColor(true).ToHex();
    TArray<FString> Tags;
    
    if (bIsBold)
    {
        Tags.Add(TEXT("b"));
    }
    
    if (FontSize > 0)
    {
        Tags.Add(FString::Printf(TEXT("font size=\"%d\""), FontSize));
    }
    
    if (!ColorHex.IsEmpty())
    {
        Tags.Add(FString::Printf(TEXT("color=\"#%s\""), *ColorHex));
    }
    
    FString OpenTags = FString::Join(Tags, TEXT(" "));
    FString FormattedText = FString::Printf(TEXT("<%s>Измерение: %dD</%s>"), *OpenTags, Dimension, *OpenTags);
    
    return FormattedText;
}

void UUIDimensionDisplay::UpdateDimensionIcon(int32 Dimension)
{
    if (!DimensionIcon)
    {
        return;
    }

    if (DimensionIcons.IsValidIndex(Dimension))
    {
        UTexture2D* Icon = DimensionIcons[Dimension];
        if (Icon)
        {
            DimensionIcon->SetBrushFromTexture(Icon, true);
            DimensionIcon->SetVisibility(ESlateVisibility::Visible);
            
            UE_LOG(LogTemp, Verbose, TEXT("UUIDimensionDisplay: Updated icon for dimension %d"), Dimension);
        }
        else
        {
            DimensionIcon->SetVisibility(ESlateVisibility::Hidden);
        }
    }
    else
    {
        DimensionIcon->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UUIDimensionDisplay::ApplyTextStyle(int32 Dimension)
{
    if (!Label)
    {
        return;
    }

    if (DimensionStyles.Contains(Dimension))
    {
        const FTextBlockStyle& Style = DimensionStyles[Dimension];
        Label->SetFont(Style.Font);
        Label->SetColorAndOpacity(Style.ColorAndOpacity);
        Label->SetShadowOffset(Style.ShadowOffset);
        Label->SetShadowColorAndOpacity(Style.ShadowColorAndOpacity);
    }
    else
    {
        FSlateFontInfo FontInfo = Label->GetFont();
        FontInfo.Size = FontSize;
        FontInfo.TypefaceFontName = bIsBold ? TEXT("Bold") : TEXT("Regular");
        Label->SetFont(FontInfo);
        Label->SetColorAndOpacity(TextColor);
    }
}

void UUIDimensionDisplay::UpdateTextColor(const FLinearColor& Color)
{
    if (Label)
    {
        Label->SetColorAndOpacity(Color);
    }
    else if (RichLabel)
    {
        FString CurrentText = RichLabel->GetText().ToString();
        FString ColorHex = Color.ToFColor(true).ToHex();
        FString NewText = FString::Printf(TEXT("<color=#%s>%s</color>"), *ColorHex, *CurrentText);
        RichLabel->SetText(FText::FromString(NewText));
    }
}

FLinearColor UUIDimensionDisplay::GetDimensionColor(int32 Dimension) const
{
    if (const FTextBlockStyle* Style = DimensionStyles.Find(Dimension))
    {
        return Style->ColorAndOpacity.GetSpecifiedColor();
    }
    return TextColor.GetSpecifiedColor();
}

void UUIDimensionDisplay::UpdateDimensionVisual(const FLinearColor& Color, float Scale, float Opacity, float Progress)
{
    if (Label)
    {
        Label->SetColorAndOpacity(FSlateColor(Color));
        Label->SetRenderOpacity(Opacity);
        Label->SetRenderScale(FVector2D(Scale));
    }

    if (RichLabel)
    {
        RichLabel->SetRenderOpacity(Opacity);
        RichLabel->SetRenderScale(FVector2D(Scale));
    }
}

static UUIDimensionDisplay* CreateDimensionWidget(UObject* WorldContextObject, TSubclassOf<UUIDimensionDisplay> WidgetClass)
{
    if (!WorldContextObject || !WidgetClass)
    {
        return nullptr;
    }

    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return nullptr;
    }

    UUIDimensionDisplay* Widget = CreateWidget<UUIDimensionDisplay>(World, WidgetClass);
    if (Widget)
    {
        Widget->AddToViewport();
        Widget->UpdateUI(0);
    }

    return Widget;
}

void UUIDimensionDisplay::SetDimensionName(const FString& Name)
{
    if (Label)
    {
        Label->SetText(FText::FromString(Name));
    }
    else if (RichLabel)
    {
        RichLabel->SetText(FText::FromString(Name));
    }
}

void UUIDimensionDisplay::SetDimensionColor(const FLinearColor& Color)
{
    if (Label)
    {
        Label->SetColorAndOpacity(Color);
    }
    else if (RichLabel)
    {
        FString CurrentText = RichLabel->GetText().ToString();
        FString ColorHex = Color.ToFColor(true).ToHex();
        FString NewText = FString::Printf(TEXT("<color=#%s>%s</color>"), *ColorHex, *CurrentText);
        RichLabel->SetText(FText::FromString(NewText));
    }
}

void UUIDimensionDisplay::SetDimensionIndex(int32 Dimension)
{
    UpdateUI(Dimension);
}

void UUIDimensionDisplay::PlayTransitionAnimation(int32 NewDimension)
{
    if (NewDimension == CurrentDimension)
    {
        return;
    }

    PreviousDimension = CurrentDimension;
    TargetDimension = NewDimension;
    AnimationTime = 0.0f;
    bIsAnimating = true;

    UpdateUI(NewDimension);
}
