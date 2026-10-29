#include "AtlantisLoreSystem.h"
#include "GameManager.h"
#include "VRUIManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Engine/Texture2D.h"

UAtlantisLoreSystem::UAtlantisLoreSystem()
{
    bRemembersAtlantis = false;
    AtlantisMemories.Empty();
    GameManager = nullptr;
    VRUIManager = nullptr;

    UE_LOG(LogTemp, Verbose, TEXT("UAtlantisLoreSystem: Constructor initialized"));
}

void UAtlantisLoreSystem::TriggerMemory()
{
    if (bRemembersAtlantis)
    {
        UE_LOG(LogTemp, Log, TEXT("UAtlantisLoreSystem: Atlantis already remembered"));
        return;
    }

    bRemembersAtlantis = true;

    if (VRUIManager)
    {
        VRUIManager->ShowMemoryFlash(GetAtlantisMemory());
    }

    if (AtlantisSound)
    {
        UGameplayStatics::PlaySound2D(GetWorld(), AtlantisSound);
    }

    if (GameManager)
    {
        GameManager->OnAtlantisRemembered();
    }

    OnAtlantisMemoryTriggered.Broadcast();

    UE_LOG(LogTemp, Log, TEXT("UAtlantisLoreSystem: Atlantis memory triggered"));
}

FAtlantisMemory UAtlantisLoreSystem::GetAtlantisMemory() const
{
    if (AtlantisMemories.IsEmpty())
    {
        return FAtlantisMemory();
    }
    return AtlantisMemories[FMath::RandRange(0, AtlantisMemories.Num() - 1)];
}