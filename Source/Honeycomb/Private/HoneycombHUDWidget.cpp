#include "HoneycombHUDWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Styling/SlateColor.h"

void UHoneycombHUDWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (!WidgetTree)
    {
        return;
    }

    RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
    WidgetTree->RootWidget = RootCanvas;

    auto AddCanvas = [this](UWidget* Widget, FAnchors Anchors, FVector2D Alignment,
                            FVector2D Position, FVector2D Size)
    {
        UCanvasPanelSlot* Slot = RootCanvas->AddChildToCanvas(Widget);
        Slot->SetAnchors(Anchors);
        Slot->SetAlignment(Alignment);
        Slot->SetPosition(Position);
        Slot->SetAutoSize(false);
        Slot->SetSize(Size);
        return Slot;
    };

    QuestTitle = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("QuestTitle"));
    QuestTitle->SetText(FText::FromString(TEXT("Quest")));
    QuestTitle->SetJustification(ETextJustify::Center);
    QuestTitle->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.95f, 0.8f, 1.0f)));
    AddCanvas(QuestTitle, FAnchors(0.5f, 0.0f, 0.5f, 0.0f), FVector2D(0.5f, 0.0f),
              FVector2D(0.0f, 24.0f), FVector2D(420.0f, 28.0f));

    QuestProgress = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("QuestProgress"));
    QuestProgress->SetPercent(0.0f);
    AddCanvas(QuestProgress, FAnchors(0.5f, 0.0f, 0.5f, 0.0f), FVector2D(0.5f, 0.0f),
              FVector2D(0.0f, 54.0f), FVector2D(420.0f, 10.0f));

    HealthBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("HealthBar"));
    HealthBar->SetPercent(1.0f);
    HealthBar->SetFillColorAndOpacity(FLinearColor(0.85f, 0.15f, 0.2f, 1.0f));
    AddCanvas(HealthBar, FAnchors(0.0f, 1.0f, 0.0f, 1.0f), FVector2D(0.0f, 1.0f),
              FVector2D(40.0f, -40.0f), FVector2D(360.0f, 18.0f));

    UOverlay* EnergyContainer = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("EnergyContainer"));
    EnergyBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("EnergyBar"));
    EnergyBar->SetPercent(1.0f);
    EnergyBar->SetFillColorAndOpacity(FLinearColor(0.2f, 0.7f, 1.0f, 1.0f));
    EnergyContainer->AddChildToOverlay(EnergyBar);
    AddCanvas(EnergyContainer, FAnchors(0.0f, 1.0f, 0.0f, 1.0f), FVector2D(0.0f, 1.0f),
              FVector2D(40.0f, -64.0f), FVector2D(360.0f, 14.0f));

    UHorizontalBox* ChakraContainer = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("ChakraContainer"));
    AddCanvas(ChakraContainer, FAnchors(0.0f, 0.0f, 0.0f, 0.0f), FVector2D(0.0f, 0.0f),
              FVector2D(24.0f, 24.0f), FVector2D(400.0f, 40.0f));

    UImage* KarmaImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("KarmaImage"));
    AddCanvas(KarmaImage, FAnchors(1.0f, 0.0f, 1.0f, 0.0f), FVector2D(1.0f, 0.0f),
              FVector2D(-24.0f, 24.0f), FVector2D(48.0f, 48.0f));

    UImage* MoodImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("MoodImage"));
    AddCanvas(MoodImage, FAnchors(1.0f, 0.0f, 1.0f, 0.0f), FVector2D(1.0f, 0.0f),
              FVector2D(-24.0f, 80.0f), FVector2D(48.0f, 48.0f));

    UVerticalBox* AbilitiesContainer = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("AbilitiesContainer"));
    AddCanvas(AbilitiesContainer, FAnchors(1.0f, 0.5f, 1.0f, 0.5f), FVector2D(1.0f, 0.5f),
              FVector2D(-24.0f, 0.0f), FVector2D(220.0f, 200.0f));

    UCanvasPanel* MeditationPanel = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("MeditationPanel"));
    AddCanvas(MeditationPanel, FAnchors(0.5f, 0.5f, 0.5f, 0.5f), FVector2D(0.5f, 0.5f),
              FVector2D(0.0f, 0.0f), FVector2D(420.0f, 80.0f));

    MeditationText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("MeditationText"));
    MeditationText->SetText(FText::FromString(TEXT("Meditation")));
    MeditationText->SetJustification(ETextJustify::Center);
    {
        UCanvasPanelSlot* TextSlot = MeditationPanel->AddChildToCanvas(MeditationText);
        TextSlot->SetAnchors(FAnchors(0.5f, 0.0f, 0.5f, 0.0f));
        TextSlot->SetAlignment(FVector2D(0.5f, 0.0f));
        TextSlot->SetPosition(FVector2D(0.0f, 0.0f));
        TextSlot->SetSize(FVector2D(420.0f, 24.0f));
    }

    MeditationProgress = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("MeditationProgress"));
    MeditationProgress->SetPercent(0.0f);
    {
        UCanvasPanelSlot* ProgressSlot = MeditationPanel->AddChildToCanvas(MeditationProgress);
        ProgressSlot->SetAnchors(FAnchors(0.5f, 0.0f, 0.5f, 0.0f));
        ProgressSlot->SetAlignment(FVector2D(0.5f, 0.0f));
        ProgressSlot->SetPosition(FVector2D(0.0f, 30.0f));
        ProgressSlot->SetSize(FVector2D(420.0f, 16.0f));
    }
    MeditationPanel->SetVisibility(ESlateVisibility::Collapsed);

    NotificationText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("NotificationText"));
    NotificationText->SetJustification(ETextJustify::Center);
    NotificationText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 1.0f, 0.85f, 1.0f)));
    AddCanvas(NotificationText, FAnchors(0.5f, 1.0f, 0.5f, 1.0f), FVector2D(0.5f, 1.0f),
              FVector2D(0.0f, -96.0f), FVector2D(600.0f, 28.0f));
}
