#include "MinimalHealthUI.h"
#include "VRPlayerController.h"
#include "Systems/DiseaseSystem.h"
#include "Systems/MoodSystem.h"
#include "GameManager.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/PanelWidget.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"

UMinimalHealthUI::UMinimalHealthUI(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
    Player = nullptr;
    DiseaseSystem = nullptr;
    MoodSystem = nullptr;
    GameManager = nullptr;
    CurrentHealthPercentage = 1.0f;
    CurrentTemperature = 36.6f;
    bSystemsFound = false;
    HealthyColor = FLinearColor::Green;
    InjuredColor = FLinearColor::Yellow;
    CriticalColor = FLinearColor::Red;
    NormalTempColor = FLinearColor::White;
    FeverTempColor = FLinearColor::Red;
    ColdTempColor = FLinearColor::Blue;
}

void UMinimalHealthUI::NativeConstruct()
{
    Super::NativeConstruct();

    if (HealthBar)
    {
        OriginalHealthBarSize = HealthBar->GetDesiredSize();
        OriginalHealthBarSize = HealthBar->GetBrush().ImageSize;
    }

    FindPlayerAndSystems();
    RefreshUI();

    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            UpdateTimerHandle,
            this,
            &UMinimalHealthUI::RefreshUI,
            0.1f,
            true
        );
    }
}

void UMinimalHealthUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!bSystemsFound)
    {
        FindPlayerAndSystems();
    }
}

void UMinimalHealthUI::NativeDestruct()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(UpdateTimerHandle);
    }

    for (UUserWidget* Effect : ActiveStatusEffects)
    {
        if (Effect)
        {
            Effect->RemoveFromParent();
        }
    }
    ActiveStatusEffects.Empty();

    Super::NativeDestruct();
}

void UMinimalHealthUI::FindPlayerAndSystems()
{
    Player = nullptr;
    GameManager = nullptr;
    DiseaseSystem = nullptr;
    MoodSystem = nullptr;
    bSystemsFound = false;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("Player character not found"));
        return;
    }

    AVRPlayerController* FoundPlayerController = Cast<AVRPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
    if (!FoundPlayerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("Player controller not found"));
        return;
    }
    Player = FoundPlayerController;

    GameManager = Cast<AGameManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameManager::StaticClass()));
    if (!GameManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("GameManager not found"));
        return;
    }

    DiseaseSystem = GameManager->GetDiseaseSystem();
    MoodSystem = GameManager->GetMoodSystem();

    if (!DiseaseSystem || !MoodSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Systems not found"));
        return;
    }

    bSystemsFound = true;
    RefreshUI();
}

void UMinimalHealthUI::RefreshUI()
{
    if (!Player)
    {
        FindPlayerAndSystems();
        return;
    }

    UpdateHealthDisplay();
    UpdateTemperatureDisplay();
    UpdateStatusEffects();
}

void UMinimalHealthUI::UpdateHealthDisplay()
{
    if (!Player) return;

    CurrentHealthPercentage = Player->GetHealthPercentage();
    float HealthPercentage = CurrentHealthPercentage;

    if (HealthBar)
    {
        FVector2D NewSize = OriginalHealthBarSize;
        NewSize.X = OriginalHealthBarSize.X * HealthPercentage;
        HealthBar->SetDesiredSizeOverride(NewSize);
        FSlateBrush Brush = HealthBar->GetBrush();
        Brush.ImageSize = NewSize;
        HealthBar->SetBrush(Brush);
        FLinearColor HealthColor = GetHealthColor(HealthPercentage);
        HealthBar->SetColorAndOpacity(HealthColor);
    }

    if (HealthText)
    {
        int32 HealthPercent = FMath::RoundToInt(HealthPercentage * 100);
        FString HealthString = FString::Printf(TEXT("%d%%"), HealthPercent);
        HealthText->SetText(FText::FromString(HealthString));
    }
}

void UMinimalHealthUI::UpdateTemperatureDisplay()
{
    if (!TemperatureText) return;

    float Temperature = 36.6f;

    if (DiseaseSystem)
    {
        Temperature = DiseaseSystem->GetBodyTemperature();
    }

    CurrentTemperature = Temperature;

    FString TempString = FString::Printf(TEXT("%.1f°C"), Temperature);
    TemperatureText->SetText(FText::FromString(TempString));

    if (TemperatureIndicator)
    {
        FLinearColor TempColor = GetTemperatureColor(Temperature);
        TemperatureIndicator->SetColorAndOpacity(TempColor);
    }
}

void UMinimalHealthUI::UpdateStatusEffects()
{
    if (!StatusEffectsContainer) return;

    StatusEffectsContainer->ClearChildren();
    
    for (UUserWidget* Effect : ActiveStatusEffects)
    {
        if (Effect)
        {
            Effect->RemoveFromParent();
        }
    }
    ActiveStatusEffects.Empty();

    if (DiseaseSystem && DiseaseSystem->HasActiveDiseases())
    {
        TArray<FDisease> ActiveDiseases = DiseaseSystem->GetActiveDiseases();
        
        for (const FDisease& D : ActiveDiseases)
        {
            AddStatusEffect(D.Name, D.Severity);
        }
    }

    if (MoodSystem && MoodSystem->HasActiveMoodEffects())
    {
        TArray<FString> ActiveMoodEffects = MoodSystem->GetActiveMoodEffects();
        
        for (const FString& EffectName : ActiveMoodEffects)
        {
            AddStatusEffect(EffectName, 0.5f);
        }
    }
}

void UMinimalHealthUI::AddStatusEffect(const FString& Name, float Severity)
{
    if (!StatusEffectsContainer || !StatusEffectWidgetClass) return;
    UUserWidget* StatusEffectWidget = CreateWidget<UUserWidget>(GetWorld(), StatusEffectWidgetClass);
    if (!StatusEffectWidget) return;
    StatusEffectsContainer->AddChild(StatusEffectWidget);
    ActiveStatusEffects.Add(StatusEffectWidget);
}

void UMinimalHealthUI::OnTemperatureChanged(float NewTemperature)
{
    CurrentTemperature = NewTemperature;
    
    if (TemperatureText)
    {
        FString TempString = FString::Printf(TEXT("%.1f°C"), NewTemperature);
        TemperatureText->SetText(FText::FromString(TempString));
    }

    if (TemperatureIndicator)
    {
        FLinearColor TempColor = GetTemperatureColor(NewTemperature);
        TemperatureIndicator->SetColorAndOpacity(TempColor);
    }

    OnTemperatureChangedEvent.Broadcast(NewTemperature);
}

void UMinimalHealthUI::OnHealthChanged(float NewHealthPercentage)
{
    CurrentHealthPercentage = NewHealthPercentage;
    if (HealthBar)
    {
        FVector2D NewSize = OriginalHealthBarSize;
        NewSize.X = OriginalHealthBarSize.X * NewHealthPercentage;
        HealthBar->SetDesiredSizeOverride(NewSize);

        FLinearColor HealthColor = GetHealthColor(NewHealthPercentage);
        HealthBar->SetColorAndOpacity(HealthColor);
    }

    if (HealthText)
    {
        int32 HealthPercent = FMath::RoundToInt(NewHealthPercentage * 100);
        FString HealthString = FString::Printf(TEXT("%d%%"), HealthPercent);
        HealthText->SetText(FText::FromString(HealthString));
    }

    OnHealthChangedEvent.Broadcast(NewHealthPercentage);
}

FLinearColor UMinimalHealthUI::GetHealthColor(float HealthPercentage) const
{
    if (HealthPercentage > 0.7f)
        return HealthyColor;
    else if (HealthPercentage > 0.3f)
        return InjuredColor;
    else
        return CriticalColor;
}

FLinearColor UMinimalHealthUI::GetTemperatureColor(float Temperature) const
{
    if (Temperature > 37.5f)
        return FeverTempColor;
    else if (Temperature < 35.5f)
        return ColdTempColor;
    else
        return NormalTempColor;
}
