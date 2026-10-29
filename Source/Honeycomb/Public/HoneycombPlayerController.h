#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HoneycombPlayerController.generated.h"

class AGameManager;
class AVRUIManager;

/**
 * Flat-screen player controller.
 *
 * Keeps the VR controller (AVRPlayerController) untouched and provides the
 * non-VR entry point: it wires the persistent managers and puts input into
 * game-only mode with the cursor hidden so WASD + mouse look feel native.
 */
UCLASS(Blueprintable, BlueprintType)
class HONEYCOMB_API AHoneycombPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AHoneycombPlayerController();

    virtual void BeginPlay() override;

    UFUNCTION(BlueprintPure, Category = "Honeycomb|Player")
    AGameManager* GetGameManager() const { return GameManager; }

    UFUNCTION(BlueprintPure, Category = "Honeycomb|Player")
    AVRUIManager* GetUIManager() const { return UIManager; }

    /** Console command helper: builds the geolocated street on demand. */
    UFUNCTION(Exec, Category = "Honeycomb|Street")
    void HoneycombBuildStreet(float RadiusMeters = 250.0f);

    UFUNCTION(Exec)
    void HoneycombBuildStreetAt(double Latitude, double Longitude, float RadiusMeters = 250.0f);

protected:
    void InitializeFlatGame();

    UPROPERTY(Transient)
    AGameManager* GameManager;

    UPROPERTY(Transient)
    AVRUIManager* UIManager;
};
