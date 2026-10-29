#pragma once

#include "HoneycombCore.h"
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "QuestSystem.generated.h"

UENUM(BlueprintType)
enum class QuestType: uint8
{
    Physical,
    Emotional,
    Mental,
    Spiritual,
    Dimensional,
    WorldSpecific,
    Chakra,
    Moon
};

UENUM(BlueprintType)
enum class QuestDifficulty: uint8
{
    None = 0,
    Easy = 1,
    Normal = 2,
    Hard = 3,
    Extreme = 4,
    Impossible = 5
};

UENUM(BlueprintType)
enum class ObjectiveType: uint8
{
    Collect,
    Kill,
    Interact,
    Travel,
    Meditate,
    Heal,
    Create,
    Destroy
};

UENUM(BlueprintType)
enum class RewardType: uint8
{
    Experience,
    ChakraEnergy,
    Items,
    Abilities,
    WorldAccess,
    DimensionShift
};

USTRUCT(BlueprintType)
struct FQuestObjective
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString Description;

    UPROPERTY(BlueprintReadWrite)
    int32 TargetAmount = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 CurrentAmount = 0;

    UPROPERTY(BlueprintReadWrite)
    bool bIsCompleted = false;

    UPROPERTY(BlueprintReadWrite)
    ObjectiveType Type = ObjectiveType::Interact;
};

USTRUCT(BlueprintType)
struct FQuestReward
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    RewardType Type = RewardType::Experience;

    UPROPERTY(BlueprintReadWrite)
    FString ItemId;

    UPROPERTY(BlueprintReadWrite)
    int32 Amount = 0;

    UPROPERTY(BlueprintReadWrite)
    float Value = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    int32 TargetWorldIndex = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 TargetDimension = 0;
};

USTRUCT(BlueprintType)
struct FQuestData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString Id;

    UPROPERTY(BlueprintReadWrite)
    FString Title;

    UPROPERTY(BlueprintReadWrite)
    FString Description;

    UPROPERTY(BlueprintReadWrite)
    QuestType Type = QuestType::Physical;

    UPROPERTY(BlueprintReadWrite)
    int32 RequiredChakraLevel = 1;

    UPROPERTY(BlueprintReadWrite)
    int32 RequiredDimension = 1;

    UPROPERTY(BlueprintReadWrite)
    TArray<FQuestObjective> Objectives;

    UPROPERTY(BlueprintReadWrite)
    TArray<FQuestReward> Rewards;

    UPROPERTY(BlueprintReadWrite)
    bool bIsCompleted = false;

    UPROPERTY(BlueprintReadWrite)
    bool bIsActive = false;

    UPROPERTY(BlueprintReadWrite)
    float TimeLimit = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    QuestDifficulty Difficulty = QuestDifficulty::Normal;
};

UCLASS(BlueprintType, Blueprintable)
class HONEYCOMB_API UQuestSystem: public UObject
{
    GENERATED_BODY()

    public:
        UQuestSystem();

        void Initialize();
        void Update(float DeltaTime);

        UFUNCTION(BlueprintCallable, Category = "Quest")
        void ActivateQuest(const FString& QuestId);

        UFUNCTION(BlueprintCallable, Category = "Quest")
        void CompleteQuest(const FString& QuestId);

        UFUNCTION(BlueprintCallable, Category = "Quest")
        void UpdateObjective(const FString& QuestId, int32 ObjectiveIndex, int32 Progress);

        UFUNCTION(BlueprintCallable, Category = "Quest")
        TArray<FQuestData> GetActiveQuests() const {
            return ActiveQuests;
        }

        UFUNCTION(BlueprintCallable, Category = "Quest")
        TArray<FQuestData> GetCompletedQuests() const {
            return CompletedQuests;
        }

        UFUNCTION(BlueprintCallable, Category = "Quest")
        FQuestData GetQuestById(const FString& QuestId) const;

        UFUNCTION(BlueprintCallable, Category = "Quest")
        bool IsQuestActive(const FString& QuestId) const;

        UFUNCTION(BlueprintCallable, Category = "Quest")
        int32 GetActiveQuestCount() const {
            return ActiveQuests.Num();
        }

        void SetGameManager(class AGameManager* GM) {
            GameManagerRef = GM;
        }

        void SetChakraSystem(class UChakraSystem* System) {
            ChakraSystemRef = System;
        }

        void SetDimensionSystem(class UDimensionSystem* System) {
            DimensionSystemRef = System;
        }
        
        void SetWorldSystem(class UWorldSystem* System) {
            WorldSystemRef = System;
        }

    private:
        void InitializeQuests();
        void CreateQuest(const FString& Id, const FString& Title, const FString& Description, QuestType Type, int32 ChakraLevel, int32 Dimension, QuestDifficulty Difficulty);
        void CreateWorldSpecificQuests();
        TArray<FQuestObjective> GenerateObjectives(QuestType Type, QuestDifficulty Difficulty);
        FString GenerateObjectiveDescription(QuestType Type, int32 Index) const;
        ObjectiveType GetObjectiveType(QuestType Type, int32 Index) const;
        TArray<FQuestReward> GenerateRewards(QuestType Type, QuestDifficulty Difficulty);
        float GetTimeLimit(QuestDifficulty Difficulty) const;
        void GenerateInitialQuests();
        void GenerateNewQuests();
        TArray<FQuestData> GetAvailableQuests() const;
        void GiveRewards(const FQuestData& Quest);
        void CheckChakraAdvancement();
        void CheckQuestCompletion();
        bool IsQuestCompleted(const FQuestData& Quest) const;

        UPROPERTY()
        TArray<FQuestData> AllQuests;

        UPROPERTY()
        TArray<FQuestData> ActiveQuests;

        UPROPERTY()
        TArray<FQuestData> CompletedQuests;

        UPROPERTY()
        class AGameManager* GameManagerRef = nullptr;

        UPROPERTY()
        class UChakraSystem* ChakraSystemRef = nullptr;

        UPROPERTY()
        class UDimensionSystem* DimensionSystemRef = nullptr;

        UPROPERTY()
        class UWorldSystem* WorldSystemRef = nullptr;

        UPROPERTY()
        int32 MaxActiveQuests = 5;

        UPROPERTY()
        float QuestGenerationInterval = 300.0f;

        float LastQuestGeneration = 0.0f;

        UPROPERTY()
        int32 CurrentQuestIndex = 0;

        UPROPERTY()
        bool bIsTracking = false;

        bool bIsInitialized = false;
};
