#pragma once

#include "HoneycombCore.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "VRPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UCameraComponent;
class UParticleSystemComponent;
class USphereComponent;
class UCharacterMovementComponent;

UCLASS()
class HONEYCOMB_API AVRPlayerController: public APlayerController
{
    GENERATED_BODY()

    public:
        AVRPlayerController();

    protected:
        virtual void BeginPlay() override;
        virtual void Tick(float DeltaTime) override;
        virtual void SetupInputComponent() override;

    public:
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "XR")
        UCameraComponent* VRCamera;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "XR")
        USphereComponent* TeleportComponent;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "XR")
        UParticleSystemComponent* TeleportVisualizer;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "XR")
        UParticleSystemComponent* MeditationParticles;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
        float WalkSpeed = 150.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
        float RunSpeed = 300.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
        float MaxHealth = 100.0f;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
        float CurrentHealth;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
        float HealthRegeneration = 1.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chakra")
        float MeditationProgress = 0.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chakra")
        float MeditationRequired = 100.0f;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chakra")
        bool bIsMeditating = false;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
        TArray<FString> UnlockedAbilities;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
        float TeleportRange = 1000.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
        float HealingPower = 10.0f;

        UPROPERTY(EditAnywhere, Category = "Input")
        UInputMappingContext* InputMappingContext;

        UPROPERTY(EditAnywhere, Category = "Input")
        UInputAction* MeditateAction;

        UPROPERTY(EditAnywhere, Category = "Input")
        UInputAction* HealAction;

        UPROPERTY(EditAnywhere, Category = "Input")
        UInputAction* TeleportAction;

        UPROPERTY(EditAnywhere, Category = "Input")
        UInputAction* ToggleRunAction;

        UPROPERTY(EditAnywhere, Category = "Keyboard")
        bool bEnableKeyboardFallbacks = true;

        UPROPERTY(EditAnywhere, Category = "Keyboard")
        FKey MeditateKey = EKeys::M;

        UPROPERTY(EditAnywhere, Category = "Keyboard")
        FKey HealKey = EKeys::H;

        UPROPERTY(EditAnywhere, Category = "Keyboard")
        FKey TeleportKey = EKeys::T;

        UPROPERTY(EditAnywhere, Category = "Keyboard")
        FKey ToggleRunKey = EKeys::R;

        UPROPERTY()
        class AGameManager* GameManager;

        UPROPERTY()
        class UChakraSystem* ChakraSystem;

        UPROPERTY()
        class UQuestSystem* QuestSystem;

        UPROPERTY()
        class UKarmaSystem* KarmaSystem;

        UPROPERTY()
        class UMoodSystem* MoodSystem;

        UPROPERTY()
        class UDiseaseSystem* DiseaseSystem;

        UPROPERTY()
        class AVRUIManager* VRUIManager;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
        class UAudioComponent* DeepMeditationSound;

        UFUNCTION(BlueprintCallable, Category = "Player")
        void ApplyPlayerDamage(float Damage);

        UFUNCTION(BlueprintCallable, Category = "Player")
        void Heal(float Amount);

        UFUNCTION(BlueprintCallable, Category = "Player")
        void UnlockAbility(const FString& AbilityName);

        UFUNCTION(BlueprintCallable, Category = "Player")
        float GetHealthPercentage() const;

        UFUNCTION(BlueprintCallable, Category = "Player")
        float GetMeditationProgress() const;

        UFUNCTION(BlueprintCallable, Category = "Teleport")
        void EnableTeleportation();

        UFUNCTION(BlueprintCallable, Category = "Teleport")
        void DisableTeleportation();

        UFUNCTION(BlueprintCallable, Category = "Movement")
        float GetMoveSpeed() const;

        UFUNCTION(BlueprintCallable, Category = "Movement")
        void SetMoveSpeedMultiplier(float Multiplier);

        UFUNCTION(BlueprintCallable, Category = "Movement")
        void SetTurnEnabled(bool bEnable);

    protected:
        void OnMeditate();
        void OnHeal();
        void OnTeleport();
        void OnToggleRun();

        void ToggleMeditation();
        void StartMeditation();
        void StopMeditation();
        void CompleteMeditation();

        void HandleMeditation(float DeltaTime);

        void ToggleRun();
        void UseHealing();
        void UseTeleport();

        void Die();

        void InitializePlayer();
        void InitializeSystems();
        void UpdateUI();

        void PositionVRUI();

        void OnHighMeditationStateEnter();
        void OnHighMeditationStateExit();

        void UpdateMeditationVFX(float Progress);

        UCharacterMovementComponent* GetCharacterMovement() const;

        bool IsRunning = false;
        bool bHighMeditationState = false;

        float MoveSpeedMultiplier = 1.0f;
        float BaseMaxWalkSpeed = 0.0f;

    private:
        void SetupInputActions();
        void CacheTeleportAction();
        void SetupProgrammaticTeleportAction();
        void EnableTeleportationComponents();
        void ActivateTeleportationFallback();
        void SetupKeyboardFallbacks();

        UPROPERTY()
        UInputAction* CachedTeleportAction;
};
