#include "HoneycombGameMode.h"

#include "Engine/GameInstance.h"
#include "GameManager.h"
#include "Geo/GeoStreetSubsystem.h"
#include "HoneycombPlayerController.h"
#include "HoneycombPlayerPawn.h"
#include "VRUIManager.h"

AHoneycombGameMode::AHoneycombGameMode()
{
    DefaultPawnClass = AHoneycombPlayerPawn::StaticClass();
    PlayerControllerClass = AHoneycombPlayerController::StaticClass();
}

void AHoneycombGameMode::StartPlay()
{
    SpawnCoreManagers();
    Super::StartPlay();

    if (bAutoBuildStreet)
    {
        BuildUserStreet(StreetRadiusMeters);
    }
}

void AHoneycombGameMode::BuildUserStreet(float RadiusMeters)
{
    UGameInstance* GameInstance = GetGameInstance();
    UGeoStreetSubsystem* Street = GameInstance
        ? GameInstance->GetSubsystem<UGeoStreetSubsystem>()
        : nullptr;

    if (!Street)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Honeycomb] No UGeoStreetSubsystem available"));
        return;
    }

    Street->BuildStreetFromUserLocation(RadiusMeters);
}

void AHoneycombGameMode::SpawnCoreManagers()
{
    GameManager = AGameManager::GetInstance();

    if (UWorld* World = GetWorld())
    {
        UIManager = AVRUIManager::GetInstance(World);
    }

    UE_LOG(LogTemp, Log,
        TEXT("[Honeycomb] Flat GameMode start. GameManager=%s UIManager=%s"),
        GameManager ? *GameManager->GetName() : TEXT("null"),
        UIManager ? *UIManager->GetName() : TEXT("null"));
}
