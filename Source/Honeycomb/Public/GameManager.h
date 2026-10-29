#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Systems/ChakraSystem.h"
#include "Systems/DimensionSystem.h"
#include "Systems/WorldSystem.h"
#include "Systems/KarmaSystem.h"
#include "Systems/ReincarnationSystem.h"
#include "Systems/MoodSystem.h"
#include "Systems/DiseaseSystem.h"
#include "Systems/QuestSystem.h"
#include "Systems/KriyaYogaSystem.h"
#include "DiseaseCatalog.h"
#include "SaveGameData.h"
#include "GameManager.generated.h"

class AVRUIManager;

UCLASS(BlueprintType, Blueprintable)
class HONEYCOMB_API AGameManager: public AActor
{
    GENERATED_BODY()

    public:
        AGameManager();

        UFUNCTION(BlueprintCallable, Category = "AGameManager")
        static AGameManager* GetInstance();

        UFUNCTION(BlueprintCallable, Category = "AGameManager|State")
        int32 GetCurrentChakraLevel() const {
            return CurrentChakraLevel;
        }

        UFUNCTION(BlueprintCallable, Category = "AGameManager|State")
        int32 GetCurrentDimension() const {
            return CurrentDimension;
        }

        UFUNCTION(BlueprintCallable, Category = "AGameManager|State")
        int32 GetCurrentWorldIndex() const {
            return CurrentWorldIndex;
        }

        UFUNCTION(BlueprintCallable, Category = "AGameManager|State")
        int32 GetTotalWorlds() const {
            return TotalWorlds;
        }

        UFUNCTION(BlueprintCallable, Category = "AGameManager|State")
        bool IsPlayerDead() const {
            return bIsDead;
        }

        /** True once the finale (12 chakras + 5th dimension) has been reached. */
        UFUNCTION(BlueprintPure, Category = "AGameManager|State")
        bool IsMoonAccessible() const {
            return CurrentChakraLevel >= TotalChakras && CurrentDimension >= MaxDimension;
        }

        void CheckMoonAccess();

        UFUNCTION(BlueprintCallable, Category = "AGameManager|State")
        FString GetChakraName(int32 ChakraLevel) const;

        UFUNCTION(BlueprintCallable, Category = "AGameManager|State")
        void SetCurrentChakraLevel(int32 NewLevel);

        UFUNCTION(BlueprintNativeEvent, Category = "UI")
        void OnUpdateUI();

        UFUNCTION(BlueprintCallable, Category = "AGameManager|Systems")
        class UChakraSystem* GetChakraSystem() const {
            return ChakraSystem;
        }

        UFUNCTION(BlueprintCallable, Category = "AGameManager|Systems")
        class UDimensionSystem* GetDimensionSystem() const {
            return DimensionSystem;
        }

        UFUNCTION(BlueprintCallable, Category = "AGameManager|Systems")
        class UWorldSystem* GetWorldSystem() const {
            return WorldSystem;
        }

        UFUNCTION(BlueprintCallable, Category = "AGameManager|Systems")
        class UKarmaSystem* GetKarmaSystem() const {
            return KarmaSystem;
        }

        UFUNCTION(BlueprintCallable, Category = "AGameManager|Systems")
        class UReincarnationSystem* GetReincarnationSystem() const {
            return ReincarnationSystem;
        }

        UFUNCTION(BlueprintCallable, Category = "AGameManager|Systems")
        class UMoodSystem* GetMoodSystem() const {
            return MoodSystem;
        }

        UFUNCTION(BlueprintCallable, Category = "AGameManager|Systems")
        class UDiseaseSystem* GetDiseaseSystem() const {
            return DiseaseSystem;
        }

        UFUNCTION(BlueprintCallable, Category = "AGameManager|Systems")
        class UQuestSystem* GetQuestSystem() const {
            return QuestSystem;
        }

        UFUNCTION(BlueprintCallable, Category = "AGameManager|Systems")
        class UKriyaYogaSystem* GetKriyaYogaSystem() const {
            return KriyaYogaSystem;
        }

        UFUNCTION(BlueprintCallable, Category = "AGameManager|Systems")
        class UDiseaseCatalog* GetDiseaseCatalog() const {
            return DiseaseCatalog;
        }

        USaveGameData* GetSaveData() const { return SaveData; }

        UFUNCTION(BlueprintCallable, Category = "AGameManager|Gameplay")
        void AdvanceChakraLevel();

        UFUNCTION(BlueprintCallable, Category = "AGameManager|Gameplay")
        void OnKundaliniAwakened() {
            if (CurrentChakraLevel < TotalChakras)
            {
                AdvanceChakraLevel();
            }
        }

        UFUNCTION(BlueprintCallable, Category = "AGameManager|Gameplay")
        void AdvanceDimension();

        UFUNCTION(BlueprintCallable, Category = "AGameManager|Gameplay")
        void OnPlayerDeath();

        UFUNCTION(BlueprintCallable, Category = "AGameManager|Gameplay")
        void OnLevelStagnation();

        UFUNCTION(BlueprintCallable, Category = "AGameManager|Gameplay")
        void ResetGame();

        UFUNCTION(BlueprintCallable, Category = "AGameManager|Gameplay")
        void OnMoonCelebrationComplete();

        UFUNCTION(BlueprintCallable, Category = "AGameManager|Gameplay")
        void OnAtlantisRemembered();

        UFUNCTION(BlueprintCallable, Category = "AGameManager|Gameplay")
        void LoadGameState();

        UFUNCTION(BlueprintCallable, Category = "AGameManager|Gameplay")
        void SaveGameState();

        UFUNCTION(BlueprintCallable, Category = "AGameManager|UI")
        void UpdateUI();

        /** Advances every gameplay system by DeltaSeconds. Normally driven by Tick; exposed for headless tests. */
        UFUNCTION(BlueprintCallable, Category = "AGameManager|Debug")
        void DebugTickSystems(float DeltaSeconds);

        /** Boots the manager and its systems without BeginPlay, for headless tests. */
        UFUNCTION(BlueprintCallable, Category = "AGameManager|Debug")
        void DebugInitialize();

        UFUNCTION(BlueprintCallable, Category = "AGameManager|Gameplay")
        int32 GetMeditationLevel() const {
            return CurrentChakraLevel;
        }

        UFUNCTION(BlueprintCallable, Category = "AGameManager|Gameplay")
        void CompleteMeditation(int32 Level) {
            OnMeditationCompleted.Broadcast(Level);
        }

        DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChakraLevelChanged, int32, NewLevel);
        DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMeditationCompleted, int32, Level);
        DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDimensionChanged, int32, NewDimension);
        DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDied);
        DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameReset);

        UPROPERTY(BlueprintAssignable, Category = "AGameManager|Events")
        FOnChakraLevelChanged OnChakraLevelChanged;

        UPROPERTY(BlueprintAssignable, Category = "AGameManager|Events")
        FOnMeditationCompleted OnMeditationCompleted;

        UPROPERTY(BlueprintAssignable, Category = "AGameManager|Events")
        FOnDimensionChanged OnDimensionChanged;

        UPROPERTY(BlueprintAssignable, Category = "AGameManager|Events")
        FOnPlayerDied OnPlayerDied;

        UPROPERTY(BlueprintAssignable, Category = "AGameManager|Events")
        FOnGameReset OnGameReset;

    protected:
        virtual void BeginPlay() override;
        virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
        virtual void Tick(float DeltaSeconds) override;

        void InitializeSystems();
        void UpdateSystems(float DeltaSeconds);
        void LoadSavedState();
        void ApplyGameState();
        void CacheUIComponents();

        int32 GetRequiredChakraForDimension(int32 Dimension) const;

    private:
        static AGameManager* Instance;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems", meta = (AllowPrivateAccess = "true"))
        UChakraSystem* ChakraSystem;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems", meta = (AllowPrivateAccess = "true"))
        UDimensionSystem* DimensionSystem;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems", meta = (AllowPrivateAccess = "true"))
        UWorldSystem* WorldSystem;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems", meta = (AllowPrivateAccess = "true"))
        UKarmaSystem* KarmaSystem;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems", meta = (AllowPrivateAccess = "true"))
        UReincarnationSystem* ReincarnationSystem;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems", meta = (AllowPrivateAccess = "true"))
        UMoodSystem* MoodSystem;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems", meta = (AllowPrivateAccess = "true"))
        UDiseaseSystem* DiseaseSystem;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems", meta = (AllowPrivateAccess = "true"))
        UQuestSystem* QuestSystem;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems", meta = (AllowPrivateAccess = "true"))
        UKriyaYogaSystem* KriyaYogaSystem;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems", meta = (AllowPrivateAccess = "true"))
        UDiseaseCatalog* DiseaseCatalog;

        UPROPERTY(SaveGame, BlueprintReadWrite, Category = "State", meta = (AllowPrivateAccess = "true"))
        int32 CurrentChakraLevel = 1;

        UPROPERTY(SaveGame, BlueprintReadWrite, Category = "State", meta = (AllowPrivateAccess = "true"))
        int32 CurrentDimension = 3;

        UPROPERTY(SaveGame, BlueprintReadWrite, Category = "State", meta = (AllowPrivateAccess = "true"))
        int32 CurrentWorldIndex = 0;

        UPROPERTY(SaveGame, BlueprintReadWrite, Category = "State", meta = (AllowPrivateAccess = "true"))
        bool bIsDead = false;

        UPROPERTY(SaveGame, BlueprintReadWrite, Category = "State", meta = (AllowPrivateAccess = "true"))
        float TotalPlayTime = 0.0f;

        static constexpr int32 TotalChakras = 12;
        static constexpr int32 TotalWorlds = 54;
        static constexpr int32 MaxDimension = 5;

        UPROPERTY()
        USaveGameData* SaveData;

        static const FString SaveSlotName;
        int32 SaveSlotIndex = 0;

        USaveGameData* CreateDefaultSaveData();
        void InitializeSaveData();
        void ApplySaveData(USaveGameData* Data);

        UPROPERTY()
        TArray<UUserWidget*> CachedUIWidgets;

        UPROPERTY(Transient)
        AVRUIManager* UIManagerRef;

        bool bLoggedFirstTick = false;

        FTimerHandle StagnationTimerHandle;
        FTimerHandle SaveTimerHandle;

        void StartStagnationTimer();
        void OnStagnationCheck();
        void AutoSave();
};
