#include "GameManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Blueprint/UserWidget.h"
#include "Systems/ChakraSystem.h"
#include "Systems/DimensionSystem.h"
#include "Systems/WorldSystem.h"
#include "Systems/KarmaSystem.h"
#include "Systems/ReincarnationSystem.h"
#include "Systems/MoodSystem.h"
#include "Systems/DiseaseSystem.h"
#include "SaveGameData.h"
#include "VRUIManager.h"

const FString AGameManager::SaveSlotName = TEXT("GameState");

AGameManager* AGameManager::Instance = nullptr;

AGameManager::AGameManager()
{
    PrimaryActorTick.bCanEverTick = true;

    USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    SetRootComponent(Root);
}

AGameManager* AGameManager::GetInstance()
{
    if (!Instance)
    {
        UWorld* World = GEngine->GetCurrentPlayWorld();
        if (World)
        {
            TArray<AActor*> FoundActors;
            UGameplayStatics::GetAllActorsOfClass(World, AGameManager::StaticClass(), FoundActors);
            if (FoundActors.Num() > 0)
            {
                Instance = Cast<AGameManager>(FoundActors[0]);
            }
            else
            {
                FActorSpawnParameters SpawnParams;
                SpawnParams.Name = TEXT("AGameManager");
                SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
                Instance = World->SpawnActor<AGameManager>(AGameManager::StaticClass(), SpawnParams);
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AGameManager::GetInstance: No valid world!"));
        }
    }
    return Instance;
}

void AGameManager::BeginPlay()
{
    Super::BeginPlay();

    if (Instance && Instance != this)
    {
        UE_LOG(LogTemp, Warning, TEXT("Duplicate AGameManager detected. Destroying..."));
        Destroy();
        return;
    }

    Instance = this;

    // Cursor ownership belongs to the player controller (flat vs VR differ);
    // the manager must not override it here.
    InitializeSystems();
    LoadGameState();
    CacheUIComponents();
    UpdateUI();

    StartStagnationTimer();

    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            SaveTimerHandle,
            this,
            &AGameManager::AutoSave,
            30.0f,
            true
        );
    }

    UE_LOG(LogTemp, Log, TEXT("AGameManager initialized successfully"));
}

void AGameManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(StagnationTimerHandle);
        GetWorld()->GetTimerManager().ClearTimer(SaveTimerHandle);
    }

    SaveGameState();

    if (Instance == this)
    {
        Instance = nullptr;
    }

    Super::EndPlay(EndPlayReason);
}

void AGameManager::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!bLoggedFirstTick)
    {
        bLoggedFirstTick = true;
        UE_LOG(LogTemp, Log, TEXT("AGameManager tick active; gameplay systems updating"));
    }

    UpdateSystems(DeltaSeconds);
}

void AGameManager::DebugTickSystems(float DeltaSeconds)
{
    UpdateSystems(DeltaSeconds);
}

void AGameManager::DebugInitialize()
{
    InitializeSystems();
    CacheUIComponents();
    UpdateUI();
}

void AGameManager::UpdateSystems(float DeltaSeconds)
{
    if (ChakraSystem)        { ChakraSystem->Update(DeltaSeconds); }
    if (WorldSystem)         { WorldSystem->Update(DeltaSeconds); }
    if (KarmaSystem)         { KarmaSystem->Update(DeltaSeconds); }
    if (ReincarnationSystem) { ReincarnationSystem->Update(DeltaSeconds); }
    if (MoodSystem)          { MoodSystem->Update(DeltaSeconds); }
    if (DiseaseSystem)       { DiseaseSystem->Update(DeltaSeconds); }
    if (QuestSystem)         { QuestSystem->Update(DeltaSeconds); }
    if (KriyaYogaSystem)     { KriyaYogaSystem->Update(DeltaSeconds); }
}

void AGameManager::InitializeSystems()
{
    if (!ChakraSystem) {
        ChakraSystem = NewObject<UChakraSystem>(this);
    }

    if (!DimensionSystem) {
        DimensionSystem = NewObject<UDimensionSystem>(this);
    }

    if (!WorldSystem) {
        WorldSystem = NewObject<UWorldSystem>(this);
    }

    if (!KarmaSystem) {
        KarmaSystem = NewObject<UKarmaSystem>(this);
    }

    if (!ReincarnationSystem) {
        ReincarnationSystem = NewObject<UReincarnationSystem>(this);
    }

    if (!MoodSystem) { MoodSystem = NewObject<UMoodSystem>(this); }
    if (!DiseaseSystem) {
        DiseaseSystem = NewObject<UDiseaseSystem>(this);
    }

    if (!QuestSystem) {
        QuestSystem = NewObject<UQuestSystem>(this);
    }

    if (!KriyaYogaSystem) {
        KriyaYogaSystem = NewObject<UKriyaYogaSystem>(this);
    }

    if (!DiseaseCatalog) {
        DiseaseCatalog = NewObject<UDiseaseCatalog>(this);
    }

    // Systems keep a back-reference to the manager; wire them before Initialize
    // so their startup logic can already query global state.
    if (ChakraSystem)
    {
        ChakraSystem->SetGameManager(this);
    }

    if (WorldSystem)
    {
        WorldSystem->SetGameManager(this);
    }

    if (KriyaYogaSystem)
    {
        KriyaYogaSystem->SetGameManager(this);
        KriyaYogaSystem->SetChakraSystem(ChakraSystem);
    }

    if (ReincarnationSystem)
    {
        ReincarnationSystem->SetGameManager(this);
        ReincarnationSystem->SetWorldSystem(WorldSystem);
        ReincarnationSystem->SetKarmaSystem(KarmaSystem);
        ReincarnationSystem->SetChakraSystem(ChakraSystem);
        ReincarnationSystem->SetDimensionSystem(DimensionSystem);
    }

    if (ChakraSystem)
    {
        ChakraSystem->Initialize();
        UE_LOG(LogTemp, Log, TEXT("ChakraSystem initialized"));
    }

    if (DimensionSystem)
    {
        DimensionSystem->InitializeDimensions();
        UE_LOG(LogTemp, Log, TEXT("DimensionSystem initialized"));
    }

    if (WorldSystem)
    {
        WorldSystem->Initialize();
        UE_LOG(LogTemp, Log, TEXT("WorldSystem initialized"));
    }

    if (KarmaSystem)
    {
        KarmaSystem->Initialize();
        UE_LOG(LogTemp, Log, TEXT("KarmaSystem initialized"));
    }

    if (ReincarnationSystem)
    {
        ReincarnationSystem->Initialize();
        UE_LOG(LogTemp, Log, TEXT("ReincarnationSystem initialized"));
    }

    if (MoodSystem)
    {
        MoodSystem->Initialize();
        UE_LOG(LogTemp, Log, TEXT("MoodSystem initialized"));
    }

    if (DiseaseSystem)
    {
        DiseaseSystem->Initialize();
        UE_LOG(LogTemp, Log, TEXT("DiseaseSystem initialized"));
    }

    if (QuestSystem)
    {
        QuestSystem->SetGameManager(this);
        QuestSystem->SetChakraSystem(ChakraSystem);
        QuestSystem->SetDimensionSystem(DimensionSystem);
        QuestSystem->SetWorldSystem(WorldSystem);
        QuestSystem->Initialize();
        UE_LOG(LogTemp, Log, TEXT("QuestSystem initialized"));
    }

    if (KriyaYogaSystem)
    {
        KriyaYogaSystem->Initialize();
        UE_LOG(LogTemp, Log, TEXT("KriyaYogaSystem initialized"));
    }

    if (DiseaseCatalog)
    {
        DiseaseCatalog->Load();
        UE_LOG(LogTemp, Log, TEXT("DiseaseCatalog initialized"));
    }
}

void AGameManager::CacheUIComponents()
{
    CachedUIWidgets.Empty();
    UIManagerRef = AVRUIManager::GetInstance(GetWorld());
}

USaveGameData* AGameManager::CreateDefaultSaveData()
{
    USaveGameData* Data = NewObject<USaveGameData>();
    Data->ResetToDefaults();
    return Data;
}

void AGameManager::InitializeSaveData()
{
    if (!SaveData)
    {
        SaveData = CreateDefaultSaveData();
    }
}

void AGameManager::ApplySaveData(USaveGameData* Data)
{
    if (!Data) return;

    CurrentChakraLevel = Data->ChakraLevel;
    CurrentDimension = Data->Dimension;
    CurrentWorldIndex = Data->WorldIndex;
    bIsDead = Data->bIsDead;
    TotalPlayTime = Data->TotalPlayTime;

    UE_LOG(LogTemp, Log, TEXT("Save data applied: Chakra=%d, Dimension=%d, World=%d"),
        CurrentChakraLevel, CurrentDimension, CurrentWorldIndex);
}

void AGameManager::LoadGameState()
{
    if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, SaveSlotIndex))
    {
        USaveGame* LoadedGame = UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveSlotIndex);
        SaveData = Cast<USaveGameData>(LoadedGame);

        if (SaveData)
        {
            UE_LOG(LogTemp, Log, TEXT("Game state loaded successfully!"));
            ApplySaveData(SaveData);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to cast save data. Creating new..."));
            SaveData = CreateDefaultSaveData();
        }
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("No saved game found. Creating new save data..."));
        SaveData = CreateDefaultSaveData();
    }

    ApplyGameState();
}

void AGameManager::SaveGameState()
{
    if (!SaveData)
    {
        SaveData = CreateDefaultSaveData();
    }

    SaveData->ChakraLevel = CurrentChakraLevel;
    SaveData->Dimension = CurrentDimension;
    SaveData->WorldIndex = CurrentWorldIndex;
    SaveData->bIsDead = bIsDead;
    SaveData->TotalPlayTime = TotalPlayTime;
    SaveData->LastSaveTime = FDateTime::Now();

    if (ChakraSystem)
    {
        SaveData->UnlockedChakras = ChakraSystem->GetUnlockedChakras();
    }

    if (KarmaSystem)
    {
        SaveData->CurrentKarma = KarmaSystem->GetKarmaValue();
    }

    if (ReincarnationSystem)
    {
        SaveData->ReincarnationCount = ReincarnationSystem->GetReincarnationCount();
    }

    UGameplayStatics::SaveGameToSlot(SaveData, SaveSlotName, SaveSlotIndex);

    UE_LOG(LogTemp, Verbose, TEXT("Game state saved successfully! Chakra=%d, Dimension=%d, Karma=%d"), SaveData->ChakraLevel, SaveData->Dimension, SaveData->CurrentKarma);
}

void AGameManager::AutoSave()
{
    SaveGameState();
    UE_LOG(LogTemp, Verbose, TEXT("Auto-save completed"));
}

void AGameManager::LoadSavedState()
{
    LoadGameState();
}

void AGameManager::ApplyGameState()
{
    UpdateUI();
}

void AGameManager::UpdateUI()
{
    for (UUserWidget* Widget : CachedUIWidgets)
    {
        if (Widget)
        {
            if (UFunction* UpdateFn = Widget->FindFunction(TEXT("UpdateUI")))
            {
                Widget->ProcessEvent(UpdateFn, nullptr);
            }
        }
    }

    if (UIManagerRef && ChakraSystem)
    {
        UIManagerRef->UpdateEnergyBar(ChakraSystem->GetEnergyPercentage());

        TArray<FChakraStatus> Statuses;
        for (int32 Level : ChakraSystem->GetUnlockedChakras())
        {
            FChakraStatus Status;
            Status.Name = GetChakraName(Level);
            Status.Value = 1.0f;
            Status.Color = ChakraSystem->GetChakraData(Level).ChakraColor;
            Statuses.Add(Status);
        }
        UIManagerRef->UpdateChakraStatus(Statuses);
    }

    UE_LOG(LogTemp, Verbose, TEXT("UI Updated"));
}

void AGameManager::StartStagnationTimer()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            StagnationTimerHandle,
            this,
            &AGameManager::OnStagnationCheck,
            30.0f,
            true
        );
    }
}

void AGameManager::OnStagnationCheck()
{
    float RandomChance = FMath::FRand();
    if (RandomChance < 0.05f)
    {
        OnLevelStagnation();
    }
}

void AGameManager::AdvanceChakraLevel()
{
    if (CurrentChakraLevel < TotalChakras)
    {
        CurrentChakraLevel++;
        UE_LOG(LogTemp, Log, TEXT("Chakra advanced to level: %d"), CurrentChakraLevel);

        if (ChakraSystem)
        {
            ChakraSystem->OnChakraLevelUp(CurrentChakraLevel);
        }

        if (CurrentDimension < MaxDimension &&
            CurrentChakraLevel >= GetRequiredChakraForDimension(CurrentDimension + 1))
        {
            AdvanceDimension();
        }

        CheckMoonAccess();
        SaveGameState();
        UpdateUI();

        OnChakraLevelChanged.Broadcast(CurrentChakraLevel);
    }
}

void AGameManager::AdvanceDimension()
{
    if (CurrentDimension < MaxDimension &&
        CurrentChakraLevel >= GetRequiredChakraForDimension(CurrentDimension + 1))
    {
        CurrentDimension++;
        UE_LOG(LogTemp, Log, TEXT("Dimension advanced to: %d"), CurrentDimension);

        if (DimensionSystem)
        {
            DimensionSystem->OnDimensionChange(CurrentDimension);
        }

        SaveGameState();
        UpdateUI();

        OnDimensionChanged.Broadcast(CurrentDimension);
    }
}

int32 AGameManager::GetRequiredChakraForDimension(int32 Dimension) const
{
    switch (Dimension)
    {
        case 1:
            return 1;
        case 2:
            return 2;
        case 3:
            return 3;
        case 4:
            return 7;
        case 5:
            return 10;
        default:
            return 12;
    }
}

void AGameManager::OnPlayerDeath()
{
    if (bIsDead)
        return;

    bIsDead = true;
    UE_LOG(LogTemp, Warning, TEXT("Player died!"));

    OnPlayerDied.Broadcast();

    if (ReincarnationSystem && ReincarnationSystem->CanReincarnate())
    {
        ReincarnationSystem->Reincarnate();
        // Reincarnation revives the player, so never leave the death flag set.
        bIsDead = false;
        SaveGameState();
        UpdateUI();
        return;
    }

    CurrentWorldIndex = (CurrentWorldIndex + 1) % TotalWorlds;

    if (WorldSystem)
    {
        WorldSystem->TransitionToWorld(CurrentWorldIndex);
    }

    bIsDead = false;
    SaveGameState();
    UpdateUI();

    UE_LOG(LogTemp, Log, TEXT("Player respawned at world: %d"), CurrentWorldIndex);
}

void AGameManager::OnLevelStagnation()
{
    if (CurrentChakraLevel > 1)
    {
        CurrentChakraLevel--;
        UE_LOG(LogTemp, Warning, TEXT("Player stagnated. Chakra decreased to %d"), CurrentChakraLevel);
        SaveGameState();
        UpdateUI();

        OnChakraLevelChanged.Broadcast(CurrentChakraLevel);
    }
}

void AGameManager::CheckMoonAccess()
{
    if (!IsMoonAccessible())
    {
        return;
    }

    // Never attempt a level travel outside a real game world (headless runs).
    UWorld* World = GetWorld();
    if (!World || !World->IsGameWorld())
    {
        UE_LOG(LogTemp, Log, TEXT("Access to Moon granted (travel suppressed outside game world)"));
        return;
    }

    if (UGameplayStatics::GetCurrentLevelName(World, true) != FName("MoonLevel"))
    {
        UGameplayStatics::OpenLevel(World, FName("MoonLevel"));
        UE_LOG(LogTemp, Log, TEXT("Access to Moon granted!"));
    }
}

void AGameManager::ResetGame()
{
    CurrentDimension = 3;
    CurrentChakraLevel = 1;
    CurrentWorldIndex = 0;
    bIsDead = false;
    TotalPlayTime = 0.0f;

    SaveGameState();
    UpdateUI();

    OnGameReset.Broadcast();

    if (GetWorld())
    {
        FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(GetWorld());
        UGameplayStatics::OpenLevel(GetWorld(), FName(*CurrentLevelName));
    }

    UE_LOG(LogTemp, Log, TEXT("Game reset completed"));
}

FString AGameManager::GetChakraName(int32 ChakraLevel) const
{
    switch (ChakraLevel)
    {
        case 1:
            return TEXT("Muladhara");
        case 2:
            return TEXT("Svadhisthana");
        case 3:
            return TEXT("Manipura");
        case 4:
            return TEXT("Anahata");
        case 5:
            return TEXT("Vishuddha");
        case 6:
            return TEXT("Ajna");
        case 7:
            return TEXT("Sahasrara");
        case 8:
            return TEXT("Stellar");
        case 9:
            return TEXT("Galactic");
        case 10:
            return TEXT("Universal");
        case 11:
            return TEXT("Divine");
        case 12:
            return TEXT("Absolute");
        default:
            return TEXT("Unknown");
    }
}

void AGameManager::SetCurrentChakraLevel(int32 NewLevel)
{
    CurrentChakraLevel = FMath::Clamp(NewLevel, 1, TotalChakras);
    UE_LOG(LogTemp, Log, TEXT("Chakra level set to: %d"), CurrentChakraLevel);
    
    if (ChakraSystem)
    {
        ChakraSystem->OnChakraLevelUp(CurrentChakraLevel);
    }
    
    if (CurrentDimension < MaxDimension &&
        CurrentChakraLevel >= GetRequiredChakraForDimension(CurrentDimension + 1))
    {
        AdvanceDimension();
    }
    
    CheckMoonAccess();
    SaveGameState();
    UpdateUI();
    
    OnChakraLevelChanged.Broadcast(CurrentChakraLevel);
}

void AGameManager::OnUpdateUI_Implementation()
{
    UE_LOG(LogTemp, Verbose, TEXT("OnUpdateUI default implementation"));
    
    for (UUserWidget* Widget : CachedUIWidgets)
    {
        if (Widget)
        {
            if (UFunction* UpdateFn = Widget->FindFunction(TEXT("UpdateUI")))
            {
                Widget->ProcessEvent(UpdateFn, nullptr);
            }
        }
    }
}

void AGameManager::OnMoonCelebrationComplete()
{
    UE_LOG(LogTemp, Log, TEXT("AGameManager: Moon celebration completed by Curator"));
    CheckMoonAccess();
    SaveGameState();
    UpdateUI();
}

void AGameManager::OnAtlantisRemembered()
{
    UE_LOG(LogTemp, Log, TEXT("AGameManager: Atlantis memory remembered by player"));
    SaveGameState();
    UpdateUI();
}
