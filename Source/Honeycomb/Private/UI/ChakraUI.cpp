#include "ChakraUI.h"
#include "Systems/ChakraSystem.h"
#include "GameManager.h"
#include "CoreMinimal.h"
#include "UIChakraDisplay.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

UChakraUI::UChakraUI(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
    ChakraSystem = nullptr;
    GameManager = nullptr;
    bIsInitialized = false;
    SelectedChakraLevel = 0;

    InitializeDefaultColors();
}

void UChakraUI::NativeConstruct()
{
    Super::NativeConstruct();

    InitializeUI();

    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            UpdateTimerHandle,
            this,
            &UChakraUI::UpdateChakraStates,
            0.1f,
            true
        );
    }

    if (ChakraInfoPanel)
    {
        ChakraInfoPanel->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UChakraUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!bIsInitialized)
    {
        InitializeUI();
        return;
    }
}

void UChakraUI::NativeDestruct()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(UpdateTimerHandle);
    }

    ChakraDisplays.Empty();

    Super::NativeDestruct();
}

void UChakraUI::InitializeUI()
{
    GameManager = Cast<AGameManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameManager::StaticClass()));
    if (GameManager)
    {
        ChakraSystem = GameManager->GetChakraSystem();
    }

    CreateChakraDisplays();

    if (GameManager)
    {
        UpdateUI(GameManager->GetCurrentChakraLevel());
    }

    bIsInitialized = true;

    UE_LOG(LogTemp, Log, TEXT("UChakraUI: Initialized successfully"));
}

void UChakraUI::CreateChakraDisplays()
{
    if (!ChakraContainer || !ChakraWidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("UChakraUI: Chakra container or widget class not assigned!"));
        return;
    }

    ChakraContainer->ClearChildren();
    ChakraDisplays.Empty();

    if (ChakraColors.Num() < NUM_CHAKRAS)
    {
        InitializeDefaultColors();
    }

    for (int32 i = 0; i < NUM_CHAKRAS; i++)
    {
        UUserWidget* ChakraWidget = CreateWidget<UUserWidget>(GetWorld(), ChakraWidgetClass);
        if (!ChakraWidget) continue;

        ChakraContainer->AddChild(ChakraWidget);
        
        UUIChakraDisplay* Display = Cast<UUIChakraDisplay>(ChakraWidget);
        if (Display)
        {
            FLinearColor Color = GetChakraColor(i);
            Display->InitializeChakra(i + 1, Color);
            ChakraDisplays.Add(Display);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("UChakraUI: Created %d chakra displays"), ChakraDisplays.Num());
}

void UChakraUI::UpdateUI(int32 CurrentLevel)
{
    if (ChakraDisplays.Num() == 0) return;

    for (int32 i = 0; i < ChakraDisplays.Num(); i++)
    {
        if (ChakraDisplays[i])
        {
            bool bIsUnlocked = (i < CurrentLevel);
            bool bIsCurrent = (i == CurrentLevel - 1);
            
            ChakraDisplays[i]->UpdateState(bIsUnlocked, bIsCurrent);
        }
    }
}

void UChakraUI::UpdateChakraStates()
{
    if (!ChakraSystem) return;

    for (int32 i = 0; i < ChakraDisplays.Num(); i++)
    {
        if (ChakraDisplays[i])
        {
            bool bIsUnlocked = ChakraSystem->IsChakraUnlocked(i + 1);
            ChakraDisplays[i]->UpdateState(bIsUnlocked, false);
        }
    }
}

void UChakraUI::OnChakraClicked(int32 ChakraLevel)
{
    SelectedChakraLevel = ChakraLevel;

    if (ChakraSystem && ChakraSystem->IsChakraUnlocked(ChakraLevel))
    {
        FChakraData SystemData = ChakraSystem->GetChakraData(ChakraLevel);
        FChakraInfoData Info;
        Info.Name = SystemData.Name;
        Info.Description = SystemData.Description;
        Info.Mantra = SystemData.Mantra;
        Info.Abilities = SystemData.Abilities;
        Info.Color = SystemData.ChakraColor;
        Info.Level = SystemData.Level;
        ShowChakraInfo(Info);
        
        OnChakraSelected.Broadcast(ChakraLevel);
        
        UE_LOG(LogTemp, Log, TEXT("UChakraUI: Chakra %d clicked"), ChakraLevel);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UChakraUI: Chakra %d is locked"), ChakraLevel);
    }
}

void UChakraUI::ShowChakraInfo(const FChakraInfoData& FChakraData)
{
    if (ChakraInfoPanel)
    {
        ChakraInfoPanel->SetVisibility(ESlateVisibility::Visible);
    }

    if (ChakraNameText)
    {
        ChakraNameText->SetText(FText::FromString(FChakraData.Name));
    }

    if (ChakraDescriptionText)
    {
        ChakraDescriptionText->SetText(FText::FromString(FChakraData.Description));
    }

    if (ChakraMantraText)
    {
        ChakraMantraText->SetText(FText::FromString(FString::Printf(TEXT("Мантра: %s"), *FChakraData.Mantra)));
    }

    if (ChakraAbilitiesText)
    {
        FString AbilitiesString = FString::Join(FChakraData.Abilities, TEXT(", "));
        ChakraAbilitiesText->SetText(FText::FromString(FString::Printf(TEXT("Способности: %s"), *AbilitiesString)));
    }

    OnChakraInfoDisplayed.Broadcast(FChakraData);

    UE_LOG(LogTemp, Log, TEXT("=== Chakra Info ==="));
    UE_LOG(LogTemp, Log, TEXT("Name: %s"), *FChakraData.Name);
    UE_LOG(LogTemp, Log, TEXT("Description: %s"), *FChakraData.Description);
    UE_LOG(LogTemp, Log, TEXT("Mantra: %s"), *FChakraData.Mantra);
    UE_LOG(LogTemp, Log, TEXT("Abilities: %s"), *FString::Join(FChakraData.Abilities, TEXT(", ")));
}

FString UChakraUI::GetChakraName(int32 Level) const
{
    static const TArray<FString> Names = {
        TEXT("Муладхара"),
        TEXT("Свадхистана"),
        TEXT("Манипура"),
        TEXT("Анахата"),
        TEXT("Вишудха"),
        TEXT("Аджна"),
        TEXT("Сахасрара"),
        TEXT("Антардхана"),
        TEXT("Парадхана"),
        TEXT("Махадхана"),
        TEXT("Парамадхана"),
        TEXT("Атмадхана")
    };

    if (Level > 0 && Level <= Names.Num())
    {
        return Names[Level - 1];
    }
    return TEXT("Неизвестно");
}

FString UChakraUI::GetChakraDescription(int32 Level) const
{
    static const TArray<FString> Descriptions = {
        TEXT("Корневая чакра. Отвечает за выживание и стабильность."),
        TEXT("Сакральная чакра. Отвечает за творчество и эмоции."),
        TEXT("Чакра солнечного сплетения. Отвечает за силу воли и уверенность."),
        TEXT("Сердечная чакра. Отвечает за любовь и сострадание."),
        TEXT("Горловая чакра. Отвечает за общение и самовыражение."),
        TEXT("Чакра третьего глаза. Отвечает за интуицию и мудрость."),
        TEXT("Коронная чакра. Отвечает за духовную связь и просветление."),
        TEXT("Чакра межпространственных путешествий."),
        TEXT("Чакра создания реальности."),
        TEXT("Чакра понимания 5D."),
        TEXT("Чакра единства с космосом."),
        TEXT("Чакра статуса Куратора.")
    };

    if (Level > 0 && Level <= Descriptions.Num())
    {
        return Descriptions[Level - 1];
    }
    return TEXT("Описание недоступно");
}

FString UChakraUI::GetChakraMantra(int32 Level) const
{
    static const TArray<FString> Mantras = {
        TEXT("LAM"),
        TEXT("VAM"),
        TEXT("RAM"),
        TEXT("YAM"),
        TEXT("HAM"),
        TEXT("OM"),
        TEXT("AUM"),
        TEXT("SOHAM"),
        TEXT("AHAM"),
        TEXT("SHIVOHAM"),
        TEXT("BRAHMOHAM"),
        TEXT("SATCHITANANDA")
    };

    if (Level > 0 && Level <= Mantras.Num())
    {
        return Mantras[Level - 1];
    }
    return TEXT("OM");
}

TArray<FString> UChakraUI::GetChakraAbilities(int32 Level) const
{
    static const TArray<TArray<FString>> Abilities = {
        {TEXT("Выживание"), TEXT("Стабильность"), TEXT("Заземление")},
        {TEXT("Творчество"), TEXT("Эмоции"), TEXT("Сексуальность")},
        {TEXT("Сила воли"), TEXT("Уверенность"), TEXT("Личная сила")},
        {TEXT("Любовь"), TEXT("Сострадание"), TEXT("Исцеление")},
        {TEXT("Общение"), TEXT("Самовыражение"), TEXT("Истина")},
        {TEXT("Интуиция"), TEXT("Мудрость"), TEXT("Ясновидение")},
        {TEXT("Духовная связь"), TEXT("Просветление"), TEXT("Единство")},
        {TEXT("Путешествия"), TEXT("Измерения"), TEXT("Перемещение")},
        {TEXT("Создание"), TEXT("Реальность"), TEXT("Манифестация")},
        {TEXT("Понимание"), TEXT("Многомерность"), TEXT("Осознание")},
        {TEXT("Единство"), TEXT("Космос"), TEXT("Бесконечность")},
        {TEXT("Управление"), TEXT("Реальность"), TEXT("Баланс")}
    };

    if (Level > 0 && Level <= Abilities.Num())
    {
        return Abilities[Level - 1];
    }
    return TArray<FString>();
}

FLinearColor UChakraUI::GetChakraColor(int32 Level) const
{
    if (Level >= 0 && Level < ChakraColors.Num())
    {
        return ChakraColors[Level];
    }
    
    return FLinearColor::Gray;
}

void UChakraUI::InitializeDefaultColors()
{
    ChakraColors.Empty();
    ChakraColors.Add(FLinearColor::Red);
    ChakraColors.Add(FLinearColor(1.0f, 0.5f, 0.0f));
    ChakraColors.Add(FLinearColor::Yellow);
    ChakraColors.Add(FLinearColor::Green);
    ChakraColors.Add(FLinearColor::Blue);
    ChakraColors.Add(FLinearColor(0.294f, 0.0f, 0.51f));
    ChakraColors.Add(FLinearColor(1.0f, 0.0f, 1.0f));
    ChakraColors.Add(FLinearColor(0.0f, 1.0f, 1.0f));
    ChakraColors.Add(FLinearColor::White);
    ChakraColors.Add(FLinearColor(1.0f, 0.843f, 0.0f));
    ChakraColors.Add(FLinearColor::Transparent);
    ChakraColors.Add(FLinearColor::White);
}
