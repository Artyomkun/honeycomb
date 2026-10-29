#include "HoneycombPlayerController.h"

#include "Engine/GameInstance.h"
#include "GameManager.h"
#include "Geo/GeoStreetSubsystem.h"
#include "HoneycombPlayerPawn.h"
#include "VRUIManager.h"

AHoneycombPlayerController::AHoneycombPlayerController()
{
    bShowMouseCursor = false;
    bEnableClickEvents = false;
    bEnableMouseOverEvents = false;
}

void AHoneycombPlayerController::BeginPlay()
{
    Super::BeginPlay();
    InitializeFlatGame();
}

void AHoneycombPlayerController::InitializeFlatGame()
{
    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);
    bShowMouseCursor = false;

    GameManager = AGameManager::GetInstance();

    if (UWorld* World = GetWorld())
    {
        UIManager = AVRUIManager::GetInstance(World);
    }

    UE_LOG(LogTemp, Log,
        TEXT("[Honeycomb] Flat controller ready. GameManager=%s UIManager=%s Pawn=%s"),
        GameManager ? *GameManager->GetName() : TEXT("null"),
        UIManager ? *UIManager->GetName() : TEXT("null"),
        GetPawn() ? *GetPawn()->GetClass()->GetName() : TEXT("null"));
}

void AHoneycombPlayerController::HoneycombBuildStreet(float RadiusMeters)
{
    UGameInstance* GameInstance = GetGameInstance();
    UGeoStreetSubsystem* Street = GameInstance
        ? GameInstance->GetSubsystem<UGeoStreetSubsystem>()
        : nullptr;

    if (!Street)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Honeycomb] No UGeoStreetSubsystem for HoneycombBuildStreet"));
        return;
    }

    Street->BuildStreetFromUserLocation(RadiusMeters);
}

void AHoneycombPlayerController::HoneycombBuildStreetAt(
    double Latitude, double Longitude, float RadiusMeters)
{
    UGameInstance* GameInstance = GetGameInstance();
    UGeoStreetSubsystem* Street = GameInstance
        ? GameInstance->GetSubsystem<UGeoStreetSubsystem>()
        : nullptr;

    if (!Street)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Honeycomb] No UGeoStreetSubsystem for HoneycombBuildStreetAt"));
        return;
    }

    Street->BuildStreetAt(Latitude, Longitude, RadiusMeters);
}
