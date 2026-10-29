#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HoneycombHUDWidget.generated.h"

class UCanvasPanel;
class UProgressBar;
class UTextBlock;

/**
 * Flat-screen HUD built entirely in C++.
 *
 * AVRUIManager binds its widgets by name (GetWidgetFromName), so this widget
 * only has to build a tree with the expected names: HealthBar, EnergyBar,
 * EnergyContainer, MeditationPanel, MeditationProgress, MeditationText,
 * ChakraContainer, KarmaImage, MoodImage, AbilitiesContainer, QuestTitle,
 * QuestProgress and NotificationText.
 *
 * It exists so the flat build has a working HUD with no WBP asset; a designer
 * can still override AVRUIManager.WidgetClass with a Widget Blueprint later.
 */
UCLASS()
class HONEYCOMB_API UHoneycombHUDWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeOnInitialized() override;

    UPROPERTY(Transient)
    UCanvasPanel* RootCanvas;

    UPROPERTY(Transient)
    UProgressBar* HealthBar;

    UPROPERTY(Transient)
    UProgressBar* EnergyBar;

    UPROPERTY(Transient)
    UProgressBar* MeditationProgress;

    UPROPERTY(Transient)
    UTextBlock* MeditationText;

    UPROPERTY(Transient)
    UTextBlock* QuestTitle;

    UPROPERTY(Transient)
    UProgressBar* QuestProgress;

    UPROPERTY(Transient)
    UTextBlock* NotificationText;
};
