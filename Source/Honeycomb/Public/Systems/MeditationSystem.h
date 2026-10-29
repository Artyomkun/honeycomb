#pragma once

#include "VRPlayerController.h"
#include "VRUIManager.h"
#include "ChakraSystem.h"
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MeditationSystem.generated.h"

class AVRPlayerController;
class UChakraSystem;
class AVRUIManager;

UCLASS(BlueprintType, Blueprintable)
class HONEYCOMB_API UMeditationSystem: public UObject
{
    GENERATED_BODY()

    public:
        UMeditationSystem();

        void Initialize();
        void Update(float DeltaTime);

        DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMeditationComplete, int32, Level);

        UPROPERTY(BlueprintAssignable, Category = "Meditation|Events")
        FOnMeditationComplete OnMeditationComplete;

        UFUNCTION(BlueprintCallable, Category = "Meditation")
        void StartMeditation(int32 Level);

        UFUNCTION(BlueprintCallable, Category = "Meditation")
        void StopMeditation();

        UFUNCTION(BlueprintCallable, Category = "Meditation")
        bool IsMeditating() const {
            return bIsMeditating;
        }

        UFUNCTION(BlueprintCallable, Category = "Meditation")
        float GetMeditationProgress() const {
            return MeditationProgress;
        }

        UFUNCTION(BlueprintCallable, Category = "Meditation")
        int32 GetCurrentMeditationLevel() const {
            return CurrentMeditationLevel;
        }

        UFUNCTION(BlueprintCallable, Category = "Meditation")
        void SetVRInput(bool bStartPressed, bool bStopPressed);

        UFUNCTION(BlueprintCallable, Category = "Meditation")
        void ShowNotification(const FString& Message);

        void SetPlayerController(AVRPlayerController* PC) {
            PlayerController = PC;
        }
        
        void SetChakraSystem(UChakraSystem* System) {
            ChakraSystemRef = System;
        }

        void SetVRUIManager(AVRUIManager* Manager) {
            VRUIManagerRef = Manager;
        }

    private:
        float CalculateMeditationSpeed() const;
        float GetEnvironmentMeditationBonus() const;
        bool IsInMeditationZone() const;
        bool IsNightTime() const;

        void ApplyMeditationEffects(bool bStarting);
        void ApplyMeditationRewards();
        void CompleteMeditationRewards();
        void CompleteMeditation();
        void UpdateMeditationUI(float Progress);
        void EnableMeditationVFX(bool bEnable);
        void PlayMeditationSound(bool bPlay);
        void PlayCompletionEffects();
        void UnlockMeditationAbility();

        UPROPERTY()
        class AVRPlayerController* PlayerController = nullptr;

        UPROPERTY()
        class UChakraSystem* ChakraSystemRef = nullptr;

        UPROPERTY()
        class AVRUIManager* VRUIManagerRef = nullptr;

        UPROPERTY()
        class UAudioComponent* MeditationAudioComponent = nullptr;

        UPROPERTY()
        bool bIsMeditating = false;

        UPROPERTY()
        int32 CurrentMeditationLevel = 0;

        UPROPERTY()
        float MeditationProgress = 0.0f;

        UPROPERTY()
        float MeditationTime = 0.0f;

        UPROPERTY()
        float OriginalMoveSpeed = 0.0f;

        UPROPERTY()
        float BaseMeditationSpeed = 0.05f;

        UPROPERTY()
        float NotificationDuration = 2.0f;

        UPROPERTY()
        float NotificationTimer = 0.0f;

        UPROPERTY()
        FString CurrentNotification;

        bool bIsInitialized = false;
};
