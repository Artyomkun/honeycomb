#include "WorldSystem.h"
#include "GameManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"

UWorldSystem::UWorldSystem()
{
}

void UWorldSystem::Initialize()
{
    if (bIsInitialized) return;

    Worlds.Empty();
    CurrentWorldIndex = 0;
    WorldStability = 100.0f;
    bIsTransitioning = false;

    InitializeWorlds();
    bIsInitialized = true;

    UE_LOG(LogTemp, Log, TEXT("UWorldSystem initialized: %d worlds"), Worlds.Num());
}

void UWorldSystem::Update(float DeltaTime)
{
    if (!bIsInitialized) return;
}

void UWorldSystem::InitializeWorlds()
{
    Worlds.Empty();
    Worlds.Reserve(54);

    for (int32 i = 0; i < 54; i++)
    {
        Worlds.Add(GenerateWorld(i));
    }
}

FWorldData UWorldSystem::GenerateWorld(int32 Index)
{
    FWorldData World;

    if (Index < 8)
    {
        World.Index = Index;
        World.Name = FString::Printf(TEXT("Земля-%d"), Index);
        World.LevelName = FName(*FString::Printf(TEXT("Earth_%d"), Index));
        World.Description = TEXT("Параллельная версия Земли в том же измерении");
        World.Type = WorldType::Normal;
        World.Difficulty = 1.0f;
        World.WorldColor = FLinearColor::Green;
        World.bIsUnlocked = (Index == 0);
        World.RequiredChakraLevel = FMath::Min(Index + 1, 12);
        World.UniqueFeatures = { TEXT("Стандартная физика"), TEXT("Обычные люди"), TEXT("Нормальное время") };
    }
    else
    {
        World.Index = Index;
        World.Name = FString::Printf(TEXT("Мир-%d"), Index);
        World.LevelName = FName(*FString::Printf(TEXT("World_%d"), Index));
        World.Description = GenerateRandomDescription();
        World.Type = static_cast<WorldType>(FMath::RandRange(0, 9));
        World.Difficulty = 1.0f + (Index % 5) * 0.5f;
        World.WorldColor = FLinearColor::MakeRandomColor();
        World.bIsUnlocked = false;
        World.RequiredChakraLevel = FMath::Min((Index % 12) + 1, 12);
        World.UniqueFeatures = GenerateRandomFeatures();
    }

    return World;
}

FString UWorldSystem::GenerateRandomDescription() const
{
    static const TArray<FString> Descriptions = {
        TEXT("Мир с уникальными физическими законами"),
        TEXT("Параллельная реальность с иными правилами"),
        TEXT("Альтернативная версия Земли"),
        TEXT("Мир, где возможно невозможное"),
        TEXT("Реальность с измененными константами"),
        TEXT("Параллельное измерение существования"),
        TEXT("Альтернативная временная линия"),
        TEXT("Мир с искаженной историей"),
        TEXT("Реальность с новыми возможностями"),
        TEXT("Параллельный поток времени")
    };

    return Descriptions[FMath::RandRange(0, Descriptions.Num() - 1)];
}

TArray<FString> UWorldSystem::GenerateRandomFeatures() const
{
    static const TArray<FString> AllFeatures = {
        TEXT("Измененная гравитация"), TEXT("Другая атмосфера"), TEXT("Иные формы жизни"),
        TEXT("Магические явления"), TEXT("Технологические чудеса"), TEXT("Временные аномалии"),
        TEXT("Пространственные искажения"), TEXT("Энергетические поля"), TEXT("Квантовые эффекты"),
        TEXT("Психические способности"), TEXT("Духовные существа"), TEXT("Астральные планы")
    };

    TArray<FString> SelectedFeatures;
    int32 FeatureCount = FMath::RandRange(2, 4);

    while (SelectedFeatures.Num() < FeatureCount)
    {
        FString Feature = AllFeatures[FMath::RandRange(0, AllFeatures.Num() - 1)];
        if (!SelectedFeatures.Contains(Feature))
        {
            SelectedFeatures.Add(Feature);
        }
    }

    return SelectedFeatures;
}

void UWorldSystem::TransitionToWorld(int32 WorldIndex)
{
    if (WorldIndex < 0 || WorldIndex >= Worlds.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("UWorldSystem: Invalid world index %d"), WorldIndex);
        return;
    }

    if (bIsTransitioning)
    {
        UE_LOG(LogTemp, Warning, TEXT("UWorldSystem: Already transitioning"));
        return;
    }

    const FWorldData& TargetWorld = Worlds[WorldIndex];

    if (!GameManagerRef)
    {
        UE_LOG(LogTemp, Warning, TEXT("UWorldSystem: AGameManager not found"));
        return;
    }

    if (GameManagerRef->GetCurrentChakraLevel() < TargetWorld.RequiredChakraLevel)
    {
        UE_LOG(LogTemp, Warning, TEXT("UWorldSystem: Insufficient chakra for world %s (need %d)"), *TargetWorld.Name, TargetWorld.RequiredChakraLevel);
        return;
    }

    bIsTransitioning = true;
    CurrentWorldIndex = WorldIndex;

    PerformWorldTransition(TargetWorld);
}

void UWorldSystem::PerformWorldTransition(const FWorldData& TargetWorld)
{
    if (TargetWorld.LevelName.IsNone())
    {
        UE_LOG(LogTemp, Error, TEXT("UWorldSystem: Target world has no level name"));
        bIsTransitioning = false;
        return;
    }

    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("UWorldSystem: No world context"));
        bIsTransitioning = false;
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("UWorldSystem: Transitioning to %s (%s)"), *TargetWorld.Name, *TargetWorld.LevelName.ToString());

    OnWorldTransition.Broadcast(TargetWorld.Index);

    UGameplayStatics::OpenLevel(GetWorld(), TargetWorld.LevelName);
}

void UWorldSystem::CreateProceduralWorld(const FWorldData& FWorldData)
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("UWorldSystem: No world context"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("UWorldSystem: Creating procedural world: %s"), *FWorldData.Name);

    FRandomStream RandomStream(FWorldData.Index);

    int32 ChunkCount = FWorldData.ChunkCount;
    float ChunkSize = FWorldData.ChunkSize;

    for (int32 i = 0; i < ChunkCount; i++)
    {
        FVector ChunkLocation = FVector(
            RandomStream.FRandRange(-1000.0f, 1000.0f),
            RandomStream.FRandRange(-1000.0f, 1000.0f),
            0.0f
        );

        SpawnChunk(ChunkLocation, ChunkSize, FWorldData.WorldColor);
    }

    SpawnProceduralObjects(RandomStream, FWorldData);

    OnWorldCreated.Broadcast(FWorldData.Index);
}

void UWorldSystem::SpawnChunk(FVector Location, float Size, FLinearColor Color)
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("UWorldSystem: No world context"));
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);

    UE_LOG(LogTemp, Log, TEXT("UWorldSystem: Spawned chunk at %s (size %.0f)"), *Location.ToString(), Size);
}

void UWorldSystem::SpawnProceduralObjects(FRandomStream& RandomStream, const FWorldData& WorldData)
{
    int32 ObjectCount = RandomStream.RandRange(3, 8);
    for (int32 i = 0; i < ObjectCount; i++)
    {
        FVector Location = FVector(
            RandomStream.FRandRange(-500.0f, 500.0f),
            RandomStream.FRandRange(-500.0f, 500.0f),
            50.0f
        );
        SpawnChunk(Location, 50.0f, WorldData.WorldColor);
    }

    UE_LOG(LogTemp, Log, TEXT("UWorldSystem: Spawned %d procedural objects in world %d"), ObjectCount, WorldData.Index);
}

FWorldData UWorldSystem::GetCurrentWorld() const
{
    if (CurrentWorldIndex >= 0 && CurrentWorldIndex < Worlds.Num())
    {
        return Worlds[CurrentWorldIndex];
    }
    return FWorldData();
}

TArray<FWorldData> UWorldSystem::GetUnlockedWorlds() const
{
    TArray<FWorldData> Result;
    for (const FWorldData& World : Worlds)
    {
        if (World.bIsUnlocked)
        {
            Result.Add(World);
        }
    }
    return Result;
}

TArray<FWorldData> UWorldSystem::GetAvailableWorlds(int32 ChakraLevel) const
{
    TArray<FWorldData> Result;
    for (const FWorldData& World : Worlds)
    {
        if (World.RequiredChakraLevel <= ChakraLevel)
        {
            Result.Add(World);
        }
    }
    return Result;
}

void UWorldSystem::UnlockWorld(int32 WorldIndex)
{
    if (WorldIndex < 0 || WorldIndex >= Worlds.Num()) return;

    if (!Worlds[WorldIndex].bIsUnlocked)
    {
        Worlds[WorldIndex].bIsUnlocked = true;
        OnWorldUnlocked.Broadcast(WorldIndex, Worlds[WorldIndex].Name);
        UE_LOG(LogTemp, Log, TEXT("UWorldSystem: World %s unlocked"), *Worlds[WorldIndex].Name);
    }
}

void UWorldSystem::ModifyWorldStability(float Amount)
{
    WorldStability = FMath::Clamp(WorldStability + Amount, 0.0f, 100.0f);
}
