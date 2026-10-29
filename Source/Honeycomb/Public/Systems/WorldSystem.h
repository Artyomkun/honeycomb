#pragma once

#include "HoneycombCore.h"
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "WorldSystem.generated.h"

UENUM(BlueprintType)
enum class WorldType: uint8
{
    Normal,
    Apocalyptic,
    Utopian,
    Dystopian,
    Technological,
    Mystical,
    Alien,
    TimeShifted,
    Dimensional,
    Quantum
};

USTRUCT(BlueprintType)
struct FWorldData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    int32 Index = 0;

    UPROPERTY(BlueprintReadWrite)
    FString Name;

    UPROPERTY(BlueprintReadWrite)
    FString Description;

    UPROPERTY(BlueprintReadWrite)
    WorldType Type = WorldType::Normal;

    UPROPERTY(BlueprintReadWrite)
    float Difficulty = 1.0f;

    UPROPERTY(BlueprintReadWrite)
    TArray<FString> UniqueFeatures;

    UPROPERTY(BlueprintReadWrite)
    FLinearColor WorldColor = FLinearColor::White;

    UPROPERTY(BlueprintReadWrite)
    bool bIsUnlocked = false;

    UPROPERTY(BlueprintReadWrite)
    int32 RequiredChakraLevel = 1;

    UPROPERTY(BlueprintReadWrite)
    FName LevelName;

    UPROPERTY(BlueprintReadWrite)
    int32 ChunkCount = 10;

    UPROPERTY(BlueprintReadWrite)
    float ChunkSize = 1000.0f;
};

UCLASS(BlueprintType, Blueprintable)
class HONEYCOMB_API UWorldSystem: public UObject
{
    GENERATED_BODY()

    public:
        UWorldSystem();

        void Initialize();
        void Update(float DeltaTime);

        UFUNCTION(BlueprintCallable, Category = "World")
        void TransitionToWorld(int32 WorldIndex);

        UFUNCTION(BlueprintCallable, Category = "World")
        FWorldData GetCurrentWorld() const;

        UFUNCTION(BlueprintCallable, Category = "World")
        TArray<FWorldData> GetAllWorlds() const {
            return Worlds;
        }

        UFUNCTION(BlueprintCallable, Category = "World")
        TArray<FWorldData> GetUnlockedWorlds() const;

        UFUNCTION(BlueprintCallable, Category = "World")
        TArray<FWorldData> GetAvailableWorlds(int32 ChakraLevel) const;

        UFUNCTION(BlueprintCallable, Category = "World")
        void UnlockWorld(int32 WorldIndex);

        UFUNCTION(BlueprintCallable, Category = "World")
        float GetWorldStability() const {
            return WorldStability;
        }

        UFUNCTION(BlueprintCallable, Category = "World")
        void ModifyWorldStability(float Amount);

        UFUNCTION(BlueprintCallable, Category = "World")
        bool IsTransitioning() const {
            return bIsTransitioning;
        }

        UFUNCTION(BlueprintCallable, Category = "World")
        int32 GetCurrentWorldIndex() const {
            return CurrentWorldIndex;
        }

        UFUNCTION(BlueprintCallable, Category = "World")
        int32 GetTotalWorlds() const { return Worlds.Num(); }

        void SetGameManager(class AGameManager* GM) {
            GameManagerRef = GM;
        }

        DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWorldTransition, int32, WorldIndex);
        DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWorldUnlocked, int32, WorldIndex, FString, WorldName);
        DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWorldCreated, int32, WorldIndex);

        UPROPERTY(BlueprintAssignable, Category = "World|Events")
        FOnWorldTransition OnWorldTransition;

        UPROPERTY(BlueprintAssignable, Category = "World|Events")
        FOnWorldUnlocked OnWorldUnlocked;

        UPROPERTY(BlueprintAssignable, Category = "World|Events")
        FOnWorldCreated OnWorldCreated;

    private:
        void InitializeWorlds();
        FWorldData GenerateWorld(int32 Index);
        FString GenerateRandomDescription() const;
        TArray<FString> GenerateRandomFeatures() const;
        void PerformWorldTransition(const FWorldData& TargetWorld);
        void CreateProceduralWorld(const FWorldData& FWorldData);
        void SpawnChunk(FVector Location, float Size, FLinearColor Color);
        void SpawnProceduralObjects(FRandomStream& RandomStream, const FWorldData& WorldData);

        UPROPERTY(BlueprintReadWrite, Category = "World", meta = (AllowPrivateAccess = "true"))
        TArray<FWorldData> Worlds;

        UPROPERTY(BlueprintReadWrite, Category = "World", meta = (AllowPrivateAccess = "true"))
        int32 CurrentWorldIndex = 0;

        UPROPERTY(BlueprintReadWrite, Category = "World", meta = (AllowPrivateAccess = "true"))
        float WorldStability = 100.0f;

        UPROPERTY(BlueprintReadWrite, Category = "World", meta = (AllowPrivateAccess = "true"))
        bool bIsTransitioning = false;

        UPROPERTY()
        class AGameManager* GameManagerRef = nullptr;

        bool bIsInitialized = false;
};
