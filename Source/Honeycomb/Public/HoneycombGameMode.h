#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HoneycombGameMode.generated.h"

class AGameManager;
class AVRUIManager;

/**
 * Non-VR game mode for the flat-screen build.
 *
 * BP_XRGameMode stays the VR entry point; this mode selects the flat pawn and
 * controller and guarantees the persistent managers exist before play starts.
 */
UCLASS(Blueprintable, BlueprintType)
class HONEYCOMB_API AHoneycombGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AHoneycombGameMode();

    virtual void StartPlay() override;

    UFUNCTION(BlueprintPure, Category = "Honeycomb|GameMode")
    AGameManager* GetGameManager() const { return GameManager; }

    UFUNCTION(BlueprintPure, Category = "Honeycomb|GameMode")
    AVRUIManager* GetUIManager() const { return UIManager; }

    /** Kicks off the runtime geolocated street build (IP -> OSM -> procedural meshes). */
    UFUNCTION(BlueprintCallable, Category = "Honeycomb|GameMode")
    void BuildUserStreet(float RadiusMeters = 250.0f);

protected:
    void SpawnCoreManagers();

    /** When true, StartPlay geolocates the user and builds their street automatically. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Honeycomb|GameMode|Street")
    bool bAutoBuildStreet = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Honeycomb|GameMode|Street")
    float StreetRadiusMeters = 250.0f;

    UPROPERTY(Transient)
    AGameManager* GameManager;

    UPROPERTY(Transient)
    AVRUIManager* UIManager;
};
