#pragma once

#include "HoneycombCore.h"
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ChakraSystem.generated.h"

USTRUCT(BlueprintType)
struct FChakraMasteryData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    int32 MeditationCount = 0;

    UPROPERTY(BlueprintReadWrite)
    float EnergyUsed = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    FDateTime UnlockTime;

    UPROPERTY(BlueprintReadWrite)
    int32 TasksCompleted = 0;

    UPROPERTY(BlueprintReadWrite)
    float MasteryProgress = 0.0f;
};

USTRUCT(BlueprintType)
struct FChakraData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    int32 Level = 0;

    UPROPERTY(BlueprintReadWrite)
    FString Name;

    UPROPERTY(BlueprintReadWrite)
    FString Description;

    UPROPERTY(BlueprintReadWrite)
    FLinearColor ChakraColor = FLinearColor::White;

    UPROPERTY(BlueprintReadWrite)
    FString Mantra;

    UPROPERTY(BlueprintReadWrite)
    TArray<FString> Abilities;

    UPROPERTY(BlueprintReadWrite)
    float EnergyMultiplier = 1.0f;

    UPROPERTY(BlueprintReadWrite)
    bool bIsUnlocked = false;
};

UCLASS(BlueprintType, Blueprintable)
class HONEYCOMB_API UChakraSystem: public UObject
{
    GENERATED_BODY()

    public:
        UChakraSystem();

        void Initialize();
        void Update(float DeltaTime);

        UFUNCTION(BlueprintCallable, Category = "Chakra")
        void InitializeChakras();

        UFUNCTION(BlueprintCallable, Category = "Chakra")
        void OnChakraLevelUp(int32 NewLevel);

        UFUNCTION(BlueprintCallable, Category = "Chakra")
        bool IsChakraUnlocked(int32 Level) const;

        UFUNCTION(BlueprintCallable, Category = "Chakra")
        FChakraData GetChakraData(int32 Level) const;

        UFUNCTION(BlueprintCallable, Category = "Chakra")
        void UseEnergy(float Amount);

        UFUNCTION(BlueprintCallable, Category = "Chakra")
        float GetEnergyPercentage() const;

        UFUNCTION(BlueprintCallable, Category = "Chakra")
        bool CanUseAbility(float EnergyCost) const;

        UFUNCTION(BlueprintCallable, Category = "Chakra")
        float GetCurrentEnergy() const { return CurrentEnergy; }

        UFUNCTION(BlueprintCallable, Category = "Chakra")
        float GetMaxEnergy() const { return MaxEnergy; }

        UFUNCTION(BlueprintCallable, Category = "Chakra")
        void SetEnergyRegenerationRate(float Rate);

        UFUNCTION(BlueprintCallable, Category = "Chakra")
        void AddEnergy(float Amount);

        UFUNCTION(BlueprintCallable, Category = "Chakra")
        void ApplyKundaliniBonus() {
            MaxEnergy += 100.0f;
            CurrentEnergy = MaxEnergy;
        }

        UFUNCTION(BlueprintCallable, Category = "Chakra")
        void SetMaxEnergy(float NewMax);

        UFUNCTION(BlueprintCallable, Category = "Chakra")
        int32 GetCurrentChakraLevel() const;

        UFUNCTION(BlueprintCallable, Category = "Chakra")
        TArray<int32> GetUnlockedChakras() const;

        UFUNCTION(BlueprintCallable, Category = "Chakra")
        float GetEnergyRegenerationRate() const;

        void SetGameManager(class AGameManager* GM) { GameManagerRef = GM; }

    private:
        bool HasMasteredPreviousChakras(int32 UpToLevel) const;
        bool IsChakraMastered(int32 Level) const;
        void ActivateChakraAbilities(const FChakraData& Chakra);
        void RegenerateEnergy(float DeltaTime);

        UPROPERTY()
        TMap<int32, FChakraMasteryData> ChakraMasteryMap;

        UPROPERTY(BlueprintReadWrite, Category = "Chakra", meta = (AllowPrivateAccess = "true"))
        TArray<FChakraData> Chakras;

        UPROPERTY(BlueprintReadWrite, Category = "Chakra", meta = (AllowPrivateAccess = "true"))
        float CurrentEnergy = 100.0f;

        UPROPERTY(BlueprintReadWrite, Category = "Chakra", meta = (AllowPrivateAccess = "true"))
        float MaxEnergy = 100.0f;

        UPROPERTY(BlueprintReadWrite, Category = "Chakra", meta = (AllowPrivateAccess = "true"))
        float EnergyRegenerationRate = 5.0f;

        UPROPERTY(BlueprintReadWrite, Category = "Chakra", meta = (AllowPrivateAccess = "true"))
        TArray<FLinearColor> ChakraColors;

        class AGameManager* GameManagerRef = nullptr;

        bool bIsInitialized = false;
};
