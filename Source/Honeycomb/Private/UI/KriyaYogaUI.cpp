#include "KriyaYogaUI.h"
#include "Systems/KriyaYogaSystem.h"
#include "GameManager.h"
#include "Components/Button.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/WrapBox.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

UKriyaYogaUI::UKriyaYogaUI(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
    KriyaSystem = nullptr;
    GameManager = nullptr;
    bIsInitialized = false;
    
    MaxEnergy = 1000.0f;
}

void UKriyaYogaUI::NativeConstruct()
{
    Super::NativeConstruct();
    InitializeUI();

    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            UpdateTimerHandle,
            this,
            &UKriyaYogaUI::RefreshUI,
            0.1f,
            true
        );
    }
}

void UKriyaYogaUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!bIsInitialized)
    {
        InitializeUI();
        return;
    }

    UpdateEnergyDisplays();
    UpdatePracticeStatus();
    UpdateChakraStatus();
}

void UKriyaYogaUI::NativeDestruct()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(UpdateTimerHandle);
    }

    KriyaDisplays.Empty();
    ChakraStatusDisplays.Empty();

    Super::NativeDestruct();
}

void UKriyaYogaUI::InitializeUI()
{
    GameManager = Cast<AGameManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameManager::StaticClass()));
    if (!GameManager)
    {
        UE_LOG(LogTemp, Error, TEXT("UKriyaYogaUI: AGameManager not found!"));
        return;
    }

    KriyaSystem = GameManager->GetKriyaYogaSystem();
    if (!KriyaSystem)
    {
        UE_LOG(LogTemp, Error, TEXT("UKriyaYogaUI: UKriyaYogaSystem not found on AGameManager!"));
        return;
    }

    CreateKriyaTechniqueDisplays();
    CreateChakraStatusDisplays();
    SetupButtons();

    bIsInitialized = true;
    RefreshUI();

    UE_LOG(LogTemp, Log, TEXT("UKriyaYogaUI: Initialized successfully"));
}

void UKriyaYogaUI::CreateKriyaTechniqueDisplays()
{
    if (!KriyaContainer || !KriyaTechniqueWidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("UKriyaYogaUI: Kriya container or widget class not assigned!"));
        return;
    }

    KriyaContainer->ClearChildren();
    KriyaDisplays.Empty();

    for (int32 i = 0; i < NUM_KRIYA_LEVELS; i++)
    {
        UUserWidget* KriyaWidget = CreateWidget<UUserWidget>(GetWorld(), KriyaTechniqueWidgetClass);
        if (!KriyaWidget) continue;

        KriyaContainer->AddChild(KriyaWidget);
        KriyaDisplays.Add(KriyaWidget);
    }

    UE_LOG(LogTemp, Log, TEXT("UKriyaYogaUI: Created %d kriya displays"), KriyaDisplays.Num());
}

void UKriyaYogaUI::CreateChakraStatusDisplays()
{
    if (!ChakraStatusContainer || !ChakraStatusWidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("UKriyaYogaUI: Chakra status container or widget class not assigned!"));
        return;
    }

    ChakraStatusContainer->ClearChildren();
    ChakraStatusDisplays.Empty();

    for (int32 i = 0; i < NUM_CHAKRAS; i++)
    {
        UUserWidget* ChakraWidget = CreateWidget<UUserWidget>(GetWorld(), ChakraStatusWidgetClass);
        if (!ChakraWidget) continue;
        ChakraStatusContainer->AddChild(ChakraWidget);
        ChakraStatusDisplays.Add(ChakraWidget);
    }

    UE_LOG(LogTemp, Log, TEXT("UKriyaYogaUI: Created %d chakra status displays"), ChakraStatusDisplays.Num());
}

void UKriyaYogaUI::SetupButtons()
{
    if (StartPracticeButton)
    {
        StartPracticeButton->OnClicked.AddDynamic(this, &UKriyaYogaUI::OnStartPracticeClicked);
    }

    if (StopPracticeButton)
    {
        StopPracticeButton->OnClicked.AddDynamic(this, &UKriyaYogaUI::OnStopPracticeClicked);
    }
}

void UKriyaYogaUI::RefreshUI()
{
    if (!KriyaSystem) return;

    UpdateEnergyDisplays();
    UpdatePracticeStatus();
    UpdateChakraStatus();

    for (UUserWidget* Display : KriyaDisplays)
    {
        if (Display)
        {
            Display->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
        }
    }
}

void UKriyaYogaUI::UpdateEnergyDisplays()
{
    if (!KriyaSystem) return;

    float KundaliniEnergy = KriyaSystem->GetKundaliniEnergy();
    float KundaliniPercent = FMath::Clamp(KundaliniEnergy / MaxEnergy, 0.0f, 1.0f);

    if (KundaliniSlider)
    {
        KundaliniSlider->SetPercent(KundaliniPercent);
    }

    if (KundaliniText)
    {
        FString Text = FString::Printf(TEXT("Кундалини: %.0f/%.0f"), KundaliniEnergy, MaxEnergy);
        KundaliniText->SetText(FText::FromString(Text));
    }

    if (KundaliniFill)
    {
        UpdateFillColor(KundaliniFill, KundaliniPercent, KundaliniLowColor, KundaliniHighColor);
    }

    float PranaEnergy = KriyaSystem->GetPranaEnergy();
    float PranaPercent = FMath::Clamp(PranaEnergy / MaxEnergy, 0.0f, 1.0f);

    if (PranaSlider)
    {
        PranaSlider->SetPercent(PranaPercent);
    }

    if (PranaText)
    {
        FString Text = FString::Printf(TEXT("Прана: %.0f/%.0f"), PranaEnergy, MaxEnergy);
        PranaText->SetText(FText::FromString(Text));
    }

    if (PranaFill)
    {
        UpdateFillColor(PranaFill, PranaPercent, PranaLowColor, PranaHighColor);
    }
}

void UKriyaYogaUI::UpdatePracticeStatus()
{
    if (!KriyaSystem) return;

    bool bIsActive = KriyaSystem->IsKriyaActive();

    if (StartPracticeButton)
    {
        StartPracticeButton->SetIsEnabled(!bIsActive);
    }

    if (StopPracticeButton)
    {
        StopPracticeButton->SetIsEnabled(bIsActive);
    }

    if (PracticeStatusText)
    {
        FString StatusText;
        if (bIsActive)
        {
            FString TechniqueName = KriyaSystem->GetCurrentTechniqueName();
            StatusText = FString::Printf(TEXT("Крийя практика: %s"), *TechniqueName);
        }
        else
        {
            StatusText = TEXT("Крийя не активна");
        }
        PracticeStatusText->SetText(FText::FromString(StatusText));
    }

    if (PracticeProgressSlider)
    {
        float Progress = KriyaSystem->GetKriyaProgress();
        PracticeProgressSlider->SetPercent(Progress);
    }
}

void UKriyaYogaUI::UpdateChakraStatus()
{
    if (!KriyaSystem) return;

    for (int32 i = 0; i < ChakraStatusDisplays.Num(); i++)
    {
        if (ChakraStatusDisplays[i])
        {
            bool bIsActivated = KriyaSystem->IsChakraActivated(i + 1);
            FWidgetTransform Transform = ChakraStatusDisplays[i]->GetRenderTransform();
            Transform.Scale = bIsActivated ? FVector2D(1.0f, 1.0f) : FVector2D(0.5f, 0.5f);
            ChakraStatusDisplays[i]->SetRenderTransform(Transform);
        }
    }
}

void UKriyaYogaUI::OnStartPracticeClicked()
{
    if (!KriyaSystem || !GameManager) return;

    int32 CurrentLevel = GameManager->GetCurrentChakraLevel();
    KriyaSystem->StartKriyaPractice(CurrentLevel);
    
    OnKriyaPracticeStarted.Broadcast();
    
    UE_LOG(LogTemp, Log, TEXT("UKriyaYogaUI: Practice started at level %d"), CurrentLevel);
}

void UKriyaYogaUI::OnStopPracticeClicked()
{
    if (KriyaSystem)
    {
        KriyaSystem->StopKriyaPractice();
        OnKriyaPracticeStopped.Broadcast();
        
        UE_LOG(LogTemp, Log, TEXT("UKriyaYogaUI: Practice stopped"));
    }
}

void UKriyaYogaUI::OnKriyaTechniqueClicked(int32 KriyaLevel)
{
    if (KriyaSystem)
    {
        KriyaSystem->StartKriyaPractice(KriyaLevel);
        OnKriyaTechniqueSelected.Broadcast(KriyaLevel);
        
        UE_LOG(LogTemp, Log, TEXT("UKriyaYogaUI: Technique selected at level %d"), KriyaLevel);
    }
}

void UKriyaYogaUI::UpdateFillColor(UImage* Fill, float Percent, const FLinearColor& LowColor, const FLinearColor& HighColor)
{
    if (!Fill) return;
    
    FLinearColor Color = FMath::Lerp(LowColor, HighColor, Percent);
    Fill->SetColorAndOpacity(Color);
}

FString UKriyaYogaUI::GetKriyaName(int32 Level) const
{
    static const TArray<FString> Names = {
        TEXT("Муладхара Крийя"),
        TEXT("Свадхистана Крийя"),
        TEXT("Манипура Крийя"),
        TEXT("Анахата Крийя"),
        TEXT("Вишудха Крийя"),
        TEXT("Аджна Крийя"),
        TEXT("Сахасрара Крийя"),
        TEXT("Антардхана Крийя"),
        TEXT("Парадхана Крийя"),
        TEXT("Махадхана Крийя"),
        TEXT("Парамадхана Крийя"),
        TEXT("Атмадхана Крийя")
    };

    if (Level > 0 && Level <= Names.Num())
    {
        return Names[Level - 1];
    }
    return TEXT("Неизвестно");
}

FString UKriyaYogaUI::GetKriyaDescription(int32 Level) const
{
    static const TArray<FString> Descriptions = {
        TEXT("Пробуждение корневой чакры"),
        TEXT("Активация сакральной энергии"),
        TEXT("Пробуждение солнечного сплетения"),
        TEXT("Открытие сердечной чакры"),
        TEXT("Активация горловой чакры"),
        TEXT("Пробуждение третьего глаза"),
        TEXT("Открытие коронной чакры"),
        TEXT("Межпространственные путешествия"),
        TEXT("Создание реальности"),
        TEXT("Понимание 5D"),
        TEXT("Единство с космосом"),
        TEXT("Статус Куратора")
    };

    if (Level > 0 && Level <= Descriptions.Num())
    {
        return Descriptions[Level - 1];
    }
    return TEXT("Описание недоступно");
}

FLinearColor UKriyaYogaUI::GetChakraColor(int32 Level) const
{
    static const TArray<FLinearColor> Colors = {
        FLinearColor::Red,
        FLinearColor(1.0f, 0.5f, 0.0f),
        FLinearColor::Yellow,
        FLinearColor::Green,
        FLinearColor::Blue,
        FLinearColor(0.294f, 0.0f, 0.51f),
        FLinearColor(1.0f, 0.0f, 1.0f),
        FLinearColor(0.0f, 1.0f, 1.0f),
        FLinearColor::White,
        FLinearColor(1.0f, 0.843f, 0.0f),
        FLinearColor::Transparent,
        FLinearColor::White
    };

    if (Level > 0 && Level <= Colors.Num())
    {
        return Colors[Level - 1];
    }
    return FLinearColor::Gray;
}
