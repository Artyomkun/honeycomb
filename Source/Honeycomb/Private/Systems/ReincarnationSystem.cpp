#include "ReincarnationSystem.h"
#include "GameManager.h"
#include "WorldSystem.h"
#include "KarmaSystem.h"
#include "ChakraSystem.h"
#include "DimensionSystem.h"
#include "Engine/Engine.h"

UReincarnationSystem::UReincarnationSystem()
{
    ReincarnationCount = 0;
}

void UReincarnationSystem::Initialize()
{
    if (bIsInitialized) return;

    ReincarnationCount = 0;
    LastKarmaTier = TEXT("Neutral");
    bCanReincarnate = true;

    bIsInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("UReincarnationSystem initialized"));
}

void UReincarnationSystem::Update(float DeltaTime)
{
    if (!bIsInitialized) return;
}

void UReincarnationSystem::Reincarnate()
{
    if (!GameManagerRef)
    {
        UE_LOG(LogTemp, Warning, TEXT("UReincarnationSystem: AGameManager not found"));
        return;
    }

    if (!bCanReincarnate)
    {
        UE_LOG(LogTemp, Warning, TEXT("UReincarnationSystem: Reincarnation is disabled"));
        return;
    }

    FString KarmaTierName = TEXT("Neutral");
    KarmaTier Tier = KarmaTier::Neutral;

    if (KarmaSystemRef)
    {
        Tier = KarmaSystemRef->GetKarmaTier();
        KarmaTierName = KarmaSystemRef->GetKarmaTierName();
    }

    LastKarmaTier = KarmaTierName;
    ReincarnationCount++;

    UE_LOG(LogTemp, Log, TEXT("Reincarnation #%d | Karma: %s"), ReincarnationCount, *LastKarmaTier);

    ApplyKarmaEffectsOnRebirth(Tier);

    int32 CurrentWorld = GameManagerRef->GetCurrentWorldIndex();
    int32 NextWorld = (CurrentWorld + 1) % GameManagerRef->GetTotalWorlds();

    if (WorldSystemRef)
    {
        WorldSystemRef->TransitionToWorld(NextWorld);
    }

    OnReincarnation.Broadcast(ReincarnationCount);
    OnReincarnationWithKarma.Broadcast(ReincarnationCount, LastKarmaTier);

    UE_LOG(LogTemp, Log, TEXT("Reincarnation complete. Next world: %d"), NextWorld);
}

void UReincarnationSystem::ApplyKarmaEffectsOnRebirth(KarmaTier Tier)
{
    if (!GameManagerRef) return;

    if (Tier == KarmaTier::Dark || Tier == KarmaTier::Shadow)
    {
        int32 CurrentLevel = GameManagerRef->GetCurrentChakraLevel();
        if (CurrentLevel > 1)
        {
            int32 NewLevel = FMath::Max(1, CurrentLevel - 1);
            UE_LOG(LogTemp, Log, TEXT("Negative karma: Chakra decreased from %d to %d"), CurrentLevel, NewLevel);
            GameManagerRef->SetCurrentChakraLevel(NewLevel);
        }
    }
    else if (Tier == KarmaTier::Light || Tier == KarmaTier::Radiant)
    {
        if (ChakraSystemRef)
        {
            ChakraSystemRef->AddEnergy(ChakraSystemRef->GetMaxEnergy());
            UE_LOG(LogTemp, Log, TEXT("Positive karma: Energy restored"));
        }

        if (DimensionSystemRef)
        {
            DimensionSystemRef->ModifyRealityStability(10.0f);
            UE_LOG(LogTemp, Log, TEXT("Positive karma: Reality stability +10"));
        }
    }
}
