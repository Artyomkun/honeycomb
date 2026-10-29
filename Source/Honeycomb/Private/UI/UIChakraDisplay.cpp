#include "UIChakraDisplay.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"

UUIChakraDisplay::UUIChakraDisplay(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
    CurrentChakraLevel = 0;
    TextPrefix = TEXT("Чакра: Уровень");
    bUseRichText = false;
    DefaultColor = FLinearColor::White;
    FontSize = 24;
    bIsBold = true;
    bShowChakraIcon = false;
}

void UUIChakraDisplay::UpdateUI(int32 ChakraLevel, const FString& UnusedParam)
{
    if (!Label && !RichLabel)
    {
        UE_LOG(LogTemp, Warning, TEXT("UUIChakraDisplay: No text widget available!"));
        return;
    }

    CurrentChakraLevel = ChakraLevel;

    if (bUseRichText && RichLabel)
    {
        FString FormattedText = FormatRichChakraText(ChakraLevel);
        RichLabel->SetText(FText::FromString(FormattedText));
    }
    else if (Label)
    {
        FString FormattedText = FormatChakraText(ChakraLevel);
        Label->SetText(FText::FromString(FormattedText));
        ApplyChakraStyle(ChakraLevel);
    }

    UpdateTextColor(ChakraLevel);

    if (bShowChakraIcon && ChakraIcon)
    {
        UpdateChakraIcon(ChakraLevel);
    }

    OnChakraLevelChanged.Broadcast(ChakraLevel);

    UE_LOG(LogTemp, Verbose, TEXT("UUIChakraDisplay: Updated to chakra level %d (UnusedParam: %s)"), ChakraLevel, *UnusedParam);
}

void UUIChakraDisplay::UpdateChakra(int32 ChakraLevel)
{
    UpdateUI(ChakraLevel, TEXT(""));
}

void UUIChakraDisplay::UpdateChakraWithType(int32 ChakraLevel, const FString& ChakraType)
{
    if (!Label && !RichLabel)
    {
        return;
    }

    CurrentChakraLevel = ChakraLevel;

    FString FormattedText = FString::Printf(TEXT("Чакра: %s Уровень %d"), *ChakraType, ChakraLevel);

    if (Label)
    {
        Label->SetText(FText::FromString(FormattedText));
    }
    else if (RichLabel)
    {
        FString ColorHex = GetChakraColor(ChakraLevel).ToFColor(true).ToHex();
        FString RichText = FString::Printf(TEXT("<color=#%s><b>Чакра: %s Уровень %d</b></color>"), *ColorHex, *ChakraType, ChakraLevel);
        RichLabel->SetText(FText::FromString(RichText));
    }

    OnChakraLevelChanged.Broadcast(ChakraLevel);
}

void UUIChakraDisplay::InitializeChakra(int32 ChakraIndexParam, const FLinearColor& Color)
{
    ChakraIndex = ChakraIndexParam;
    DisplayColor = Color;

    if (Label)
    {
        Label->SetColorAndOpacity(FSlateColor(Color));
    }
}

void UUIChakraDisplay::UpdateState(bool bIsUnlocked, bool bIsCurrent)
{
    FLinearColor TargetColor;
    
    if (!bIsUnlocked)
    {
        TargetColor = FLinearColor(0.25f, 0.25f, 0.25f, 0.6f);
    }
    else if (bIsCurrent)
    {
        TargetColor = FLinearColor::White;
    }
    else
    {
        TargetColor = DisplayColor;
    }

    if (Label)
    {
        Label->SetColorAndOpacity(FSlateColor(TargetColor));
        Label->SetVisibility(bIsUnlocked ? ESlateVisibility::Visible : ESlateVisibility::HitTestInvisible);
    }

    if (RichLabel)
    {
        RichLabel->SetDefaultColorAndOpacity(TargetColor);
        RichLabel->SetRenderOpacity(bIsUnlocked ? 1.0f : 0.4f);
    }

    if (bIsCurrent && bIsUnlocked)
    {
        StartPulseAnimation();
    }
    else
    {
        StopPulseAnimation();
    }
}

FString UUIChakraDisplay::FormatChakraText(int32 ChakraLevel) const
{
    return FString::Printf(TEXT("%s %d"), *TextPrefix, ChakraLevel);
}

FString UUIChakraDisplay::FormatRichChakraText(int32 ChakraLevel) const
{
    FLinearColor Color = GetChakraColor(ChakraLevel);
    FString ColorHex = Color.ToFColor(true).ToHex();
    FString BoldTag = bIsBold ? TEXT("b") : TEXT("");
    FString SizeTag = FString::Printf(TEXT("font size=\"%d\""), FontSize);
    FString ColorTag = FString::Printf(TEXT("color=\"#%s\""), *ColorHex);
    FString OpenTags = FString::Printf(TEXT("%s %s %s"), *BoldTag, *SizeTag, *ColorTag);
    OpenTags = OpenTags.TrimStartAndEnd();
    FString FormattedText;
    if (!OpenTags.IsEmpty())
    {
        FormattedText = FString::Printf(TEXT("<%s>%s %d</%s>"), *OpenTags, *TextPrefix, ChakraLevel, *OpenTags);
    }
    else
    {
        FormattedText = FString::Printf(TEXT("%s %d"), *TextPrefix, ChakraLevel);
    }
    
    return FormattedText;
}

void UUIChakraDisplay::UpdateTextColor(int32 ChakraLevel)
{
    FLinearColor Color = GetChakraColor(ChakraLevel);
    
    if (Label)
    {
        Label->SetColorAndOpacity(FSlateColor(Color));
    }
    
    if (RichLabel)
    {
        RichLabel->SetDefaultColorAndOpacity(Color);
    }
}

FLinearColor UUIChakraDisplay::GetChakraColor(int32 ChakraLevel) const
{
    if (!ChakraColors.IsValidIndex(ChakraLevel))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid ChakraLevel: %d"), ChakraLevel);
        return DefaultColor;
    }
    
    return ChakraColors[ChakraLevel];
}

void UUIChakraDisplay::ApplyChakraStyle(int32 ChakraLevel)
{
    if (!Label)
    {
        return;
    }

    FSlateFontInfo FontInfo = Label->GetFont();
    FontInfo.Size = FontSize;
    
    if (bIsBold)
    {
        FontInfo.TypefaceFontName = TEXT("Bold");
    }
    else
    {
        FontInfo.TypefaceFontName = TEXT("Regular");
    }
    
    Label->SetFont(FontInfo);
    
    UpdateTextColor(ChakraLevel);
}

void UUIChakraDisplay::UpdateChakraIcon(int32 ChakraLevel)
{
    if (!ChakraIcon)
    {
        return;
    }

    if (ChakraIcons.Contains(ChakraLevel))
    {
        UTexture2D* Icon = ChakraIcons[ChakraLevel];
        if (Icon)
        {
            ChakraIcon->SetBrushFromTexture(Icon, true);
            ChakraIcon->SetVisibility(ESlateVisibility::Visible);
            
            UE_LOG(LogTemp, Verbose, TEXT("UUIChakraDisplay: Updated icon for level %d"), ChakraLevel);
        }
        else
        {
            ChakraIcon->SetVisibility(ESlateVisibility::Hidden);
        }
    }
    else
    {
        ChakraIcon->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UUIChakraDisplay::SetTextPrefix(const FString& NewPrefix)
{
    TextPrefix = NewPrefix;
    
    if (Label || RichLabel)
    {
        UpdateUI(CurrentChakraLevel, TEXT(""));
    }
}

void UUIChakraDisplay::AddLevelColor(int32 ChakraLevel, FLinearColor Color)
{
    LevelColors.Add(ChakraLevel, Color);
    
    if (Label || RichLabel)
    {
        UpdateUI(CurrentChakraLevel, TEXT(""));
    }
}

void UUIChakraDisplay::AddLevelIcon(int32 ChakraLevel, UTexture2D* Icon)
{
    if (Icon)
    {
        ChakraIcons.Add(ChakraLevel, Icon);
        bShowChakraIcon = true;
        
        if (Label || RichLabel)
        {
            UpdateUI(CurrentChakraLevel, TEXT(""));
        }
    }
}

void UUIChakraDisplay::StartPulseAnimation()
{
    if (Label)
    {
        Label->SetRenderOpacity(1.0f);
    }
}

void UUIChakraDisplay::StopPulseAnimation()
{
    if (Label)
    {
        Label->SetRenderOpacity(1.0f);
    }
}