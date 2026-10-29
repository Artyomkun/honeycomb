#include "VRUIManager.h"
#include "HoneycombAbilityIndicator.h"
#include "HoneycombChakraIndicator.h"
#include "HoneycombHUDWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "Components/HorizontalBox.h"
#include "Components/VerticalBox.h"
#include "Components/Overlay.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"

AVRUIManager* AVRUIManager::Instance = nullptr;

AVRUIManager::AVRUIManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Default to the code-only HUD so the flat build has UI without a WBP.
    // A designer can point WidgetClass at a Widget Blueprint instead.
    WidgetClass = UHoneycombHUDWidget::StaticClass();
    ChakraIndicatorClass = UHoneycombChakraIndicator::StaticClass();
    AbilityIndicatorClass = UHoneycombAbilityIndicator::StaticClass();
}

AVRUIManager* AVRUIManager::GetInstance(UWorld* World)
{
    if (!Instance && World)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, AVRUIManager::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            Instance = Cast<AVRUIManager>(FoundActors[0]);
        }
        else
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Name = TEXT("AVRUIManager");
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            Instance = World->SpawnActor<AVRUIManager>(AVRUIManager::StaticClass(), SpawnParams);
        }
    }
    return Instance;
}

void AVRUIManager::BeginPlay()
{
    Super::BeginPlay();

    if (Instance && Instance != this)
    {
        Destroy();
        return;
    }

    Instance = this;
    CreateUI();
}

void AVRUIManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (Instance == this)
    {
        Instance = nullptr;
    }
    Super::EndPlay(EndPlayReason);
}

void AVRUIManager::CreateUI()
{
    if (!WidgetClass || !GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("AVRUIManager: WidgetClass or World is null!"));
        return;
    }

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("AVRUIManager: PlayerController not found!"));
        return;
    }

    MainWidget = CreateWidget<UUserWidget>(PC, WidgetClass);
    if (!MainWidget)
    {
        UE_LOG(LogTemp, Warning, TEXT("AVRUIManager: Failed to create widget!"));
        return;
    }

    MainWidget->AddToViewport();
    MainWidget->SetVisibility(ESlateVisibility::Visible);
    InitializeWidgetReferences();

    UE_LOG(LogTemp, Log, TEXT("AVRUIManager initialized"));
}

void AVRUIManager::InitializeWidgetReferences()
{
    if (!MainWidget) return;

    auto FindWidget = [this](const FString& Name) -> UWidget*
    {
        return MainWidget->GetWidgetFromName(FName(*Name));
    };

    HealthBar = Cast<UProgressBar>(FindWidget("HealthBar"));
    EnergyBar = Cast<UProgressBar>(FindWidget("EnergyBar"));
    EnergyContainer = Cast<UOverlay>(FindWidget("EnergyContainer"));
    MeditationPanel = Cast<UCanvasPanel>(FindWidget("MeditationPanel"));
    MeditationProgress = Cast<UProgressBar>(FindWidget("MeditationProgress"));
    MeditationText = Cast<UTextBlock>(FindWidget("MeditationText"));
    ChakraContainer = Cast<UHorizontalBox>(FindWidget("ChakraContainer"));
    KarmaImage = Cast<UImage>(FindWidget("KarmaImage"));
    MoodImage = Cast<UImage>(FindWidget("MoodImage"));
    AbilitiesContainer = Cast<UVerticalBox>(FindWidget("AbilitiesContainer"));
    QuestTitle = Cast<UTextBlock>(FindWidget("QuestTitle"));
    QuestProgress = Cast<UProgressBar>(FindWidget("QuestProgress"));
    NotificationText = Cast<UTextBlock>(FindWidget("NotificationText"));

    if (MeditationPanel)
    {
        MeditationPanel->SetVisibility(ESlateVisibility::Collapsed);
    }

    UE_LOG(LogTemp, Log, TEXT("UI References initialized"));
}

void AVRUIManager::UpdateHealthBar(float Percentage)
{
    if (HealthBar)
    {
        HealthBar->SetPercent(FMath::Clamp(Percentage, 0.0f, 1.0f));
    }
}

void AVRUIManager::UpdateEnergyBar(float Percentage)
{
    if (EnergyBar)
    {
        EnergyBar->SetPercent(FMath::Clamp(Percentage, 0.0f, 1.0f));
    }
}

void AVRUIManager::SetEnergyBarVisible(bool bVisible)
{
    if (EnergyContainer)
    {
        EnergyContainer->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}

void AVRUIManager::ShowMeditationProgress(float Progress)
{
    if (MeditationPanel)
    {
        MeditationPanel->SetVisibility(ESlateVisibility::Visible);
    }

    if (MeditationProgress)
    {
        MeditationProgress->SetPercent(FMath::Clamp(Progress, 0.0f, 1.0f));
    }

    if (MeditationText)
    {
        MeditationText->SetText(FText::FromString(FString::Printf(TEXT("Meditation: %.1f%%"), Progress * 100.0f)));
    }
}

void AVRUIManager::HideMeditationProgress()
{
    if (MeditationPanel)
    {
        MeditationPanel->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void AVRUIManager::ShowMeditationVFX()
{
    if (MeditationPanel)
    {
        MeditationPanel->SetVisibility(ESlateVisibility::Visible);
    }
    UE_LOG(LogTemp, Log, TEXT("[VRUI] Meditation VFX shown"));
}

void AVRUIManager::HideMeditationVFX()
{
    if (MeditationPanel)
    {
        MeditationPanel->SetVisibility(ESlateVisibility::Collapsed);
    }
    UE_LOG(LogTemp, Log, TEXT("[VRUI] Meditation VFX hidden"));
}

void AVRUIManager::UpdateChakraStatus(const TArray<FChakraStatus>& Status)
{
    if (!ChakraContainer) return;

    for (auto& Pair : ChakraIndicators)
    {
        if (Pair.Value)
        {
            ChakraContainer->RemoveChild(Pair.Value);
            Pair.Value->RemoveFromParent();
        }
    }
    ChakraIndicators.Empty();

    for (const FChakraStatus& StatusData : Status)
    {
        if (!StatusData.Name.IsEmpty())
        {
            AddChakraIndicator(StatusData.Name, StatusData.Color);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Chakra status updated: %d chakras"), Status.Num());
}

void AVRUIManager::AddChakraIndicator(const FString& Name, FLinearColor Color)
{
    if (!ChakraContainer || !ChakraIndicatorClass) return;

    UUserWidget* Indicator = CreateWidget<UUserWidget>(GetWorld(), ChakraIndicatorClass);
    if (Indicator)
    {
        ChakraContainer->AddChild(Indicator);
        ChakraIndicators.Add(Name, Indicator);

        UImage* IconImage = Cast<UImage>(Indicator->GetWidgetFromName(TEXT("ChakraIcon")));
        if (IconImage)
        {
            IconImage->SetColorAndOpacity(Color);
        }
    }
}

void AVRUIManager::UpdateKarmaIndicator(int32 Index)
{
    if (!KarmaImage || KarmaTextures.Num() == 0) return;

    Index = FMath::Clamp(Index, 0, KarmaTextures.Num() - 1);
    KarmaImage->SetBrushFromTexture(KarmaTextures[Index]);
}

void AVRUIManager::UpdateMoodIndicator(EMoodIndicator Mood)
{
    if (!MoodImage || MoodTextures.Num() == 0) return;

    int32 Index = FMath::Clamp((int32)Mood, 0, MoodTextures.Num() - 1);
    MoodImage->SetBrushFromTexture(MoodTextures[Index]);
}

void AVRUIManager::UpdateAbilitiesList(const TArray<FString>& Abilities)
{
    if (!AbilitiesContainer) return;

    AbilitiesContainer->ClearChildren();
    AbilityIndicators.Empty();

    for (const FString& Ability : Abilities)
    {
        AddAbility(Ability);
    }
}

void AVRUIManager::AddAbility(const FString& AbilityName)
{
    if (!AbilitiesContainer || !AbilityIndicatorClass) return;
    if (AbilityIndicators.Contains(AbilityName)) return;

    UUserWidget* Indicator = CreateWidget<UUserWidget>(GetWorld(), AbilityIndicatorClass);
    if (Indicator)
    {
        AbilitiesContainer->AddChild(Indicator);
        AbilityIndicators.Add(AbilityName, Indicator);

        UTextBlock* NameText = Cast<UTextBlock>(Indicator->GetWidgetFromName(TEXT("AbilityName")));
        if (NameText)
        {
            NameText->SetText(FText::FromString(AbilityName));
        }
    }
}

void AVRUIManager::RemoveAbility(const FString& AbilityName)
{
    if (!AbilityIndicators.Contains(AbilityName)) return;

    UUserWidget* Indicator = AbilityIndicators[AbilityName];
    if (Indicator)
    {
        AbilitiesContainer->RemoveChild(Indicator);
        Indicator->RemoveFromParent();
    }

    AbilityIndicators.Remove(AbilityName);
}

void AVRUIManager::UpdateQuestInfo(const FString& Title, float Progress)
{
    if (QuestTitle)
    {
        QuestTitle->SetText(FText::FromString(Title));
    }

    if (QuestProgress)
    {
        QuestProgress->SetPercent(FMath::Clamp(Progress, 0.0f, 1.0f));
    }
}

void AVRUIManager::ShowUI(bool bShow)
{
    bIsUIVisible = bShow;
    if (MainWidget)
    {
        MainWidget->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}

void AVRUIManager::ToggleUI()
{
    ShowUI(!bIsUIVisible);
}

void AVRUIManager::ShowNotification(const FString& Message)
{
    if (NotificationText)
    {
        NotificationText->SetText(FText::FromString(Message));
    }
    UE_LOG(LogTemp, Log, TEXT("[VRUI] %s"), *Message);
}

void AVRUIManager::ShowMemoryFlash(const FAtlantisMemory& Memory)
{
    FString Title = Memory.Title.ToString();
    FString Description = Memory.Description.ToString();

    ShowNotification(TEXT("Atlantis Memory: ") + Title);

    UE_LOG(LogTemp, Log, TEXT("[VRUI] Memory flash: %s - %s"), *Title, *Description);
}
