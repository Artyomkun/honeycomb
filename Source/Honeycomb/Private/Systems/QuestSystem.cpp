#include "QuestSystem.h"
#include "GameManager.h"
#include "ChakraSystem.h"
#include "DimensionSystem.h"
#include "WorldSystem.h"
#include "Engine/Engine.h"

UQuestSystem::UQuestSystem()
{
    ActiveQuests.Empty();
    CompletedQuests.Empty();
    CurrentQuestIndex = 0;
    bIsTracking = false;

    ChakraSystemRef = nullptr;
    WorldSystemRef = nullptr;
    DimensionSystemRef = nullptr;

    UE_LOG(LogTemp, Verbose, TEXT("UQuestSystem: Constructor initialized"));
}

void UQuestSystem::Initialize()
{
    if (bIsInitialized) return;

    ActiveQuests.Empty();
    CompletedQuests.Empty();

    InitializeQuests();
    GenerateInitialQuests();
    LastQuestGeneration = 0.0f;

    bIsInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("UQuestSystem initialized: %d quests available"), AllQuests.Num());
}

void UQuestSystem::Update(float DeltaTime)
{
    if (!bIsInitialized) return;

    CheckQuestCompletion();

    LastQuestGeneration += DeltaTime;
    if (LastQuestGeneration >= QuestGenerationInterval)
    {
        GenerateNewQuests();
        LastQuestGeneration = 0.0f;
    }
}

void UQuestSystem::InitializeQuests()
{
    CreateQuest(TEXT("PHY_001"), TEXT("Первые шаги"), TEXT("Изучите основы физического мира"), QuestType::Physical, 1, 1, QuestDifficulty::Easy);
    CreateQuest(TEXT("PHY_002"), TEXT("Выносливость"), TEXT("Пробегите 1 км без остановки"), QuestType::Physical, 2, 1, QuestDifficulty::Normal);
    CreateQuest(TEXT("PHY_003"), TEXT("Сила воли"), TEXT("Поднимите тяжелый предмет 10 раз"), QuestType::Physical, 3, 2, QuestDifficulty::Normal);
    CreateQuest(TEXT("EMO_001"), TEXT("Сердечная чакра"), TEXT("Помогите 5 людям в беде"), QuestType::Emotional, 4, 3, QuestDifficulty::Hard);
    CreateQuest(TEXT("EMO_002"), TEXT("Творчество"), TEXT("Создайте произведение искусства"), QuestType::Emotional, 5, 3, QuestDifficulty::Normal);
    CreateQuest(TEXT("EMO_003"), TEXT("Интуиция"), TEXT("Примите 10 правильных решений подряд"), QuestType::Emotional, 6, 3, QuestDifficulty::Hard);
    CreateQuest(TEXT("MEN_001"), TEXT("Мудрость"), TEXT("Изучите древний текст"), QuestType::Mental, 7, 4, QuestDifficulty::Hard);
    CreateQuest(TEXT("MEN_002"), TEXT("Просветление"), TEXT("Проведите 24 часа в медитации"), QuestType::Mental, 8, 4, QuestDifficulty::Extreme);
    CreateQuest(TEXT("MEN_003"), TEXT("Создание реальности"), TEXT("Измените физический закон"), QuestType::Mental, 9, 4, QuestDifficulty::Impossible);
    CreateQuest(TEXT("SPI_001"), TEXT("5D понимание"), TEXT("Осознайте все измерения"), QuestType::Spiritual, 10, 5, QuestDifficulty::Extreme);
    CreateQuest(TEXT("SPI_002"), TEXT("Единство"), TEXT("Соединитесь с космическим сознанием"), QuestType::Spiritual, 11, 5, QuestDifficulty::Impossible);
    CreateQuest(TEXT("SPI_003"), TEXT("Кураторство"), TEXT("Достигните Луны и встретьтесь с Куратором"), QuestType::Spiritual, 12, 5, QuestDifficulty::Impossible);
    CreateQuest(TEXT("DIM_001"), TEXT("2D переход"), TEXT("Перейдите в двумерное измерение"), QuestType::Dimensional, 2, 2, QuestDifficulty::Normal);
    CreateQuest(TEXT("DIM_002"), TEXT("4D время"), TEXT("Управляйте временем"), QuestType::Dimensional, 7, 4, QuestDifficulty::Hard);
    CreateQuest(TEXT("DIM_003"), TEXT("5D дух"), TEXT("Достигните духовного измерения"), QuestType::Dimensional, 10, 5, QuestDifficulty::Extreme);
    CreateWorldSpecificQuests();
}

void UQuestSystem::CreateQuest(const FString& Id, const FString& Title, const FString& Description, QuestType Type, int32 ChakraLevel, int32 Dimension, QuestDifficulty Difficulty)
{
    FQuestData Quest;
    Quest.Id = Id;
    Quest.Title = Title;
    Quest.Description = Description;
    Quest.Type = Type;
    Quest.RequiredChakraLevel = ChakraLevel;
    Quest.RequiredDimension = Dimension;
    Quest.Difficulty = Difficulty;
    Quest.bIsCompleted = false;
    Quest.bIsActive = false;
    Quest.TimeLimit = GetTimeLimit(Difficulty);
    Quest.Objectives = GenerateObjectives(Type, Difficulty);
    Quest.Rewards = GenerateRewards(Type, Difficulty);
    AllQuests.Add(Quest);
}

void UQuestSystem::CreateWorldSpecificQuests()
{
    CreateQuest(TEXT("WORLD_APO_001"), TEXT("Выживание"), TEXT("Выживите в постапокалиптическом мире"), QuestType::WorldSpecific, 2, 2, QuestDifficulty::Hard);
    CreateQuest(TEXT("WORLD_UTOP_001"), TEXT("Гармония"), TEXT("Достигните полной гармонии с миром"), QuestType::WorldSpecific, 3, 3, QuestDifficulty::Normal);
    CreateQuest(TEXT("WORLD_TECH_001"), TEXT("Кибернетика"), TEXT("Станьте киборгом"), QuestType::WorldSpecific, 4, 3, QuestDifficulty::Hard);
    CreateQuest(TEXT("WORLD_MYST_001"), TEXT("Магия"), TEXT("Изучите 10 заклинаний"), QuestType::WorldSpecific, 5, 3, QuestDifficulty::Hard);
    CreateQuest(TEXT("WORLD_ALIEN_001"), TEXT("Контакт"), TEXT("Установите связь с инопланетянами"), QuestType::WorldSpecific, 6, 3, QuestDifficulty::Extreme);
    CreateQuest(TEXT("WORLD_TIME_001"), TEXT("Временная петля"), TEXT("Выйдите из временной петли"), QuestType::WorldSpecific, 7, 4, QuestDifficulty::Extreme);
    CreateQuest(TEXT("WORLD_QUANT_001"), TEXT("Квантовая суперпозиция"), TEXT("Существуйте в двух состояниях одновременно"), QuestType::WorldSpecific, 8, 4, QuestDifficulty::Impossible);
}

TArray<FQuestObjective> UQuestSystem::GenerateObjectives(QuestType Type, QuestDifficulty Difficulty)
{
    TArray<FQuestObjective> Objectives;
    int32 ObjectiveCount = static_cast<int32>(Difficulty);

    for (int32 i = 0; i < ObjectiveCount; i++)
    {
        FQuestObjective Objective;
        Objective.Description = GenerateObjectiveDescription(Type, i);
        Objective.TargetAmount = FMath::RandRange(1, 10) * static_cast<int32>(Difficulty);
        Objective.CurrentAmount = 0;
        Objective.bIsCompleted = false;
        Objective.Type = GetObjectiveType(Type, i);
        Objectives.Add(Objective);
    }

    return Objectives;
}

FString UQuestSystem::GenerateObjectiveDescription(QuestType Type, int32 Index) const
{
    static const TArray<FString> PhysicalTasks = { TEXT("Пробежать"), TEXT("Поднять"), TEXT("Проплыть"), TEXT("Прыгнуть"), TEXT("Присесть") };
    static const TArray<FString> EmotionalTasks = { TEXT("Помочь"), TEXT("Пожалеть"), TEXT("Поддержать"), TEXT("Выслушать"), TEXT("Утешить") };
    static const TArray<FString> MentalTasks = { TEXT("Изучить"), TEXT("Решить"), TEXT("Понять"), TEXT("Запомнить"), TEXT("Анализировать") };
    static const TArray<FString> SpiritualTasks = { TEXT("Медитировать"), TEXT("Молиться"), TEXT("Созерцать"), TEXT("Размышлять"), TEXT("Практиковать") };

    switch (Type)
    {
        case QuestType::Physical:
            return FString::Printf(TEXT("%s %d раз"), *PhysicalTasks[Index % PhysicalTasks.Num()], FMath::RandRange(1, 20));
        case QuestType::Emotional:
            return FString::Printf(TEXT("%s %d человек"), *EmotionalTasks[Index % EmotionalTasks.Num()], FMath::RandRange(1, 10));
        case QuestType::Mental:
            return FString::Printf(TEXT("%s %d предметов"), *MentalTasks[Index % MentalTasks.Num()], FMath::RandRange(1, 5));
        case QuestType::Spiritual:
            return FString::Printf(TEXT("%s %d часов"), *SpiritualTasks[Index % SpiritualTasks.Num()], FMath::RandRange(1, 12));
        default:
            return FString::Printf(TEXT("Выполнить задание %d"), Index + 1);
    }
}

ObjectiveType UQuestSystem::GetObjectiveType(QuestType Type, int32 Index) const
{
    switch (Type)
    {
        case QuestType::Physical:
            return ObjectiveType::Travel;
        case QuestType::Emotional:
            return ObjectiveType::Interact;
        case QuestType::Mental:
            return ObjectiveType::Collect;
        case QuestType::Spiritual:
            return ObjectiveType::Meditate;
        default:
            return ObjectiveType::Interact;
    }
}

TArray<FQuestReward> UQuestSystem::GenerateRewards(QuestType Type, QuestDifficulty Difficulty)
{
    TArray<FQuestReward> Rewards;

    FQuestReward ExpReward;
    ExpReward.Type = RewardType::Experience;
    ExpReward.Amount = static_cast<int32>(Difficulty) * 100;
    Rewards.Add(ExpReward);

    FQuestReward EnergyReward;
    EnergyReward.Type = RewardType::ChakraEnergy;
    EnergyReward.Amount = static_cast<int32>(Difficulty) * 50;
    Rewards.Add(EnergyReward);

    switch (Type)
    {
        case QuestType::Chakra:
        {
            FQuestReward AbilityReward;
            AbilityReward.Type = RewardType::Abilities;
            AbilityReward.ItemId = TEXT("ChakraBoost");
            AbilityReward.Amount = 1;
            Rewards.Add(AbilityReward);
            break;
        }
        case QuestType::Dimensional:
        {
            FQuestReward DimReward;
            DimReward.Type = RewardType::DimensionShift;
            DimReward.ItemId = TEXT("DimensionKey");
            DimReward.TargetDimension = 2;
            DimReward.Amount = 1;
            Rewards.Add(DimReward);
            break;
        }
        case QuestType::WorldSpecific:
        {
            FQuestReward WorldReward;
            WorldReward.Type = RewardType::WorldAccess;
            WorldReward.ItemId = TEXT("WorldPortal");
            WorldReward.TargetWorldIndex = 2;
            WorldReward.Amount = 1;
            Rewards.Add(WorldReward);
            break;
        }
        default:
            break;
    }

    return Rewards;
}

float UQuestSystem::GetTimeLimit(QuestDifficulty Difficulty) const
{
    switch (Difficulty)
    {
        case QuestDifficulty::Easy:
            return 60.0f;
        case QuestDifficulty::Normal:
            return 180.0f;
        case QuestDifficulty::Hard:
            return 360.0f;
        case QuestDifficulty::Extreme:
            return 720.0f;
        case QuestDifficulty::Impossible:
            return 1440.0f;
        default:
            return 0.0f;
    }
}

TArray<FQuestData> UQuestSystem::GetAvailableQuests() const
{
    TArray<FQuestData> Available;
    int32 CurrentChakra = GameManagerRef ? GameManagerRef->GetCurrentChakraLevel() : 1;
    int32 CurrentDimension = GameManagerRef ? GameManagerRef->GetCurrentDimension() : 1;

    for (const FQuestData& Quest : AllQuests)
    {
        if (!Quest.bIsActive && !Quest.bIsCompleted &&
            Quest.RequiredChakraLevel <= CurrentChakra &&
            Quest.RequiredDimension <= CurrentDimension)
        {
            Available.Add(Quest);
        }
    }

    return Available;
}

void UQuestSystem::GenerateInitialQuests()
{
    TArray<FQuestData> Available = GetAvailableQuests();
    int32 Count = FMath::Min(MaxActiveQuests, Available.Num());

    for (int32 i = 0; i < Count; i++)
    {
        ActivateQuest(Available[i].Id);
    }
}

void UQuestSystem::GenerateNewQuests()
{
    if (ActiveQuests.Num() >= MaxActiveQuests) return;

    TArray<FQuestData> Available = GetAvailableQuests();
    TArray<FQuestData> DifficultQuests;

    for (const FQuestData& Quest : Available)
    {
        if (!Quest.bIsActive && !Quest.bIsCompleted && (Quest.Difficulty == QuestDifficulty::Hard || Quest.Difficulty == QuestDifficulty::Extreme || Quest.Difficulty == QuestDifficulty::Impossible))
        {
            DifficultQuests.Add(Quest);
        }
    }

    if (DifficultQuests.Num() > 0)
    {
        int32 Index = FMath::RandRange(0, DifficultQuests.Num() - 1);
        ActivateQuest(DifficultQuests[Index].Id);
    }
}

void UQuestSystem::ActivateQuest(const FString& QuestId)
{
    for (FQuestData& Quest : AllQuests)
    {
        if (Quest.Id == QuestId && !Quest.bIsActive && !Quest.bIsCompleted)
        {
            Quest.bIsActive = true;
            ActiveQuests.Add(Quest);
            UE_LOG(LogTemp, Log, TEXT("Quest activated: %s"), *Quest.Title);
            return;
        }
    }
}

void UQuestSystem::CompleteQuest(const FString& QuestId)
{
    for (int32 i = 0; i < ActiveQuests.Num(); i++)
    {
        if (ActiveQuests[i].Id == QuestId)
        {
            FQuestData Quest = ActiveQuests[i];
            Quest.bIsCompleted = true;
            Quest.bIsActive = false;

            GiveRewards(Quest);

            CompletedQuests.Add(Quest);
            ActiveQuests.RemoveAt(i);

            CheckChakraAdvancement();

            UE_LOG(LogTemp, Log, TEXT("Quest completed: %s"), *Quest.Title);
            return;
        }
    }
}

void UQuestSystem::UpdateObjective(const FString& QuestId, int32 ObjectiveIndex, int32 Progress)
{
    for (FQuestData& Quest : ActiveQuests)
    {
        if (Quest.Id == QuestId && ObjectiveIndex < Quest.Objectives.Num())
        {
            FQuestObjective& Objective = Quest.Objectives[ObjectiveIndex];
            Objective.CurrentAmount += Progress;

            if (Objective.CurrentAmount >= Objective.TargetAmount)
            {
                Objective.bIsCompleted = true;
                UE_LOG(LogTemp, Log, TEXT("Objective completed: %s"), *Objective.Description);
            }
            return;
        }
    }
}

FQuestData UQuestSystem::GetQuestById(const FString& QuestId) const
{
    for (const FQuestData& Quest : AllQuests)
    {
        if (Quest.Id == QuestId) return Quest;
    }
    return FQuestData();
}

bool UQuestSystem::IsQuestActive(const FString& QuestId) const
{
    for (const FQuestData& Quest : ActiveQuests)
    {
        if (Quest.Id == QuestId) return true;
    }
    return false;
}

void UQuestSystem::GiveRewards(const FQuestData& Quest)
{
    if (Quest.Rewards.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("UQuestSystem: Quest '%s' has no rewards"), *Quest.Title);
        return;
    }

    for (const FQuestReward& Reward : Quest.Rewards)
    {
        switch (Reward.Type)
        {
            case RewardType::ChakraEnergy:
                if (ChakraSystemRef)
                {
                    ChakraSystemRef->AddEnergy(Reward.Amount);
                    UE_LOG(LogTemp, Log, TEXT("UQuestSystem: Added %d chakra energy"), Reward.Amount);
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("UQuestSystem: ChakraSystem not found"));
                }
                break;

            case RewardType::WorldAccess:
                if (WorldSystemRef)
                {
                    WorldSystemRef->UnlockWorld(Reward.TargetWorldIndex);
                    UE_LOG(LogTemp, Log, TEXT("UQuestSystem: Unlocked world %d"), Reward.TargetWorldIndex);
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("UQuestSystem: WorldSystem not found"));
                }
                break;

            case RewardType::DimensionShift:
                if (DimensionSystemRef)
                {
                    DimensionSystemRef->ActivateShift(Reward.TargetDimension);
                    UE_LOG(LogTemp, Log, TEXT("UQuestSystem: Activated dimension shift to %d"), Reward.TargetDimension);
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("UQuestSystem: DimensionSystem not found"));
                }
                break;

            default:
                UE_LOG(LogTemp, Warning, TEXT("UQuestSystem: Unknown reward type"));
                break;
        }
    }
}

void UQuestSystem::CheckChakraAdvancement()
{
    if (!GameManagerRef) return;

    int32 CompletedCount = CompletedQuests.Num();
    int32 CurrentLevel = GameManagerRef->GetCurrentChakraLevel();

    if (CompletedCount >= CurrentLevel * 5)
    {
        GameManagerRef->AdvanceChakraLevel();
    }
}

void UQuestSystem::CheckQuestCompletion()
{
    for (int32 i = ActiveQuests.Num() - 1; i >= 0; i--)
    {
        if (IsQuestCompleted(ActiveQuests[i]))
        {
            CompleteQuest(ActiveQuests[i].Id);
        }
    }
}

bool UQuestSystem::IsQuestCompleted(const FQuestData& Quest) const
{
    for (const FQuestObjective& Objective : Quest.Objectives)
    {
        if (!Objective.bIsCompleted) return false;
    }
    return true;
}
