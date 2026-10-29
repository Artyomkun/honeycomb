#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "HoneycombPlayerPawn.generated.h"

class AHoneycombAvatar;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;

/**
 * Flat-screen player pawn used to run the game on a laptop without a headset.
 *
 * The VR path keeps BP_XRPawn; this pawn is the non-VR equivalent so the whole
 * Honeycomb loop can be played and tested with keyboard and mouse.
 *
 * Input is built at runtime as UInputAction / UInputMappingContext objects
 * rather than assets, so the flat controls exist without WBP/IMC content and
 * can be verified headlessly. WASD moves, mouse looks, E interacts, M meditates,
 * Space jumps, Left Shift runs.
 */
UCLASS(Blueprintable, BlueprintType)
class HONEYCOMB_API AHoneycombPlayerPawn : public ACharacter
{
    GENERATED_BODY()

public:
    AHoneycombPlayerPawn();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    UFUNCTION(BlueprintCallable, Category = "Honeycomb|Player")
    void SetRunHeld(bool bHeld);

    /** Meditate action the GameManager/controller can listen to. */
    UFUNCTION(BlueprintCallable, Category = "Honeycomb|Player")
    void BeginInteract();

    UFUNCTION(BlueprintCallable, Category = "Honeycomb|Player")
    void EndInteract();

    /** Actor under the interaction crosshair, or null. */
    UFUNCTION(BlueprintPure, Category = "Honeycomb|Player")
    AActor* GetFocusedActor() const { return FocusedActor; }

    UFUNCTION(BlueprintPure, Category = "Honeycomb|Player")
    bool IsInteracting() const { return bInteracting; }

    /** Optional third-person body. Left empty for the first-person flat view. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Honeycomb|Player")
    TSubclassOf<AHoneycombAvatar> AvatarClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Honeycomb|Player")
    float WalkSpeed = 320.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Honeycomb|Player")
    float RunSpeed = 620.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Honeycomb|Player")
    float InteractRange = 500.0f;

protected:
    void OnMoveForward(const FInputActionValue& Value);
    void OnMoveRight(const FInputActionValue& Value);
    void OnTurn(const FInputActionValue& Value);
    void OnLookUp(const FInputActionValue& Value);
    void OnRunStarted();
    void OnRunStopped();
    void OnInteractPressed();
    void OnMeditatePressed();

    void BuildFlatInput();

    void UpdateFocus();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Honeycomb|Player")
    UCameraComponent* FirstPersonCamera;

    UPROPERTY(Transient)
    UInputMappingContext* FlatContext;

    UPROPERTY(Transient)
    UInputAction* MoveForwardAction;

    UPROPERTY(Transient)
    UInputAction* MoveRightAction;

    UPROPERTY(Transient)
    UInputAction* TurnAction;

    UPROPERTY(Transient)
    UInputAction* LookUpAction;

    UPROPERTY(Transient)
    UInputAction* JumpAction;

    UPROPERTY(Transient)
    UInputAction* InteractAction;

    UPROPERTY(Transient)
    UInputAction* RunAction;

    UPROPERTY(Transient)
    UInputAction* MeditateAction;

    UPROPERTY(Transient)
    AHoneycombAvatar* SpawnedAvatar;

    UPROPERTY(Transient)
    AActor* FocusedActor;

    bool bRunHeld = false;
    bool bInteracting = false;
    bool bInputBuilt = false;
    bool bMeditating = false;
};
