#pragma once

#include "HoneycombCore.h"
#include "GameFramework/Actor.h"
#include "MoonScene.generated.h"

class UStaticMeshComponent;
class UAudioComponent;
class UWidgetComponent;
class UTextRenderComponent;
class UPointLightComponent;

UCLASS(Blueprintable, BlueprintType)
class HONEYCOMB_API AMoonScene: public AActor
{
    GENERATED_BODY()

    public:
        AMoonScene();

    protected:
        virtual void BeginPlay() override;
        virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
        virtual void Tick(float DeltaTime) override;

    public:
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
        AActor* MoonSurface;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
        AActor* Stars;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
        AActor* Earth;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
        AActor* Portal;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curator")
        AActor* Curator;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curator")
        FTransform CuratorPosition;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curator")
        UMaterialInterface* CuratorMaterial;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
        AActor* ChallengeOrb;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
        TArray<AActor*> ChallengePositions;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
        float ChallengeDuration = 60.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
        float ChallengeProximityRadius = 300.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
        UUserWidget* MoonUI;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
        UUserWidget* ChallengeUI;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
        UUserWidget* VictoryUI;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
        UTextRenderComponent* DialogueText;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
        UAudioComponent* AmbientAudio;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
        UAudioComponent* CuratorVoice;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
        TArray<USoundBase*> CuratorDialogues;

        UFUNCTION(BlueprintCallable, Category = "Moon")
        void ReturnToMainWorld();

        UFUNCTION(BlueprintCallable, Category = "Moon")
        void CreateNewWorld();

        DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCelebrationFinished);

        UPROPERTY(BlueprintAssignable, Category = "Moon|Events")
        FOnCelebrationFinished OnCelebrationFinished;

    protected:
        void InitializeMoonScene();
        void SetupMoonEnvironment();

        void CreateCrater(const FVector& Position, float Radius);
        void CreateCurator();

        UFUNCTION()
        void SetupFinalChallenge();

        void StartMoonSequence();
        void CuratorGreeting();
        void StartFinalChallenges();

        void AnimateCuratorIntro();

        void MeditationChallenge();
        void RealityCreationChallenge();
        void CosmicUnityChallenge();

        void CreateRealityObject();
        bool IsPlayerNearOrb() const;
        void Celebration();

        UFUNCTION()
        void UpdateChallenge(float DeltaTime);

        void AnimateChallengeOrb(AActor* Orb, float DeltaTime);
        void AnimateRealityObject(AActor* Object, float DeltaTime);
        void AnimatePortal(AActor* PortalActor, float DeltaTime);

        UFUNCTION()
        void OnMeditationComplete();

        UFUNCTION()
        void OnRealityCreationComplete();

        UFUNCTION()
        void OnCosmicUnityComplete();

        UFUNCTION()
        void OnCelebrationComplete();

        void StopCelebrationEffects();

    private:
        class AGameManager* GameManager;

        UPROPERTY()
        class USceneComponent* NextSceneTrigger;

        bool bIsChallengeActive = false;
        bool bIsCuratorMet = false;
        float ChallengeTimer = 0.0f;
        int32 CurrentChallenge = 0;

        FTimerHandle MeditationTimerHandle;
        FTimerHandle RealityCreationTimerHandle;
        FTimerHandle CosmicUnityTimerHandle;
        FTimerHandle CelebrationTimerHandle;
        FTimerHandle CuratorIntroTimerHandle;
        FTimerHandle PortalAnimationTimerHandle;

        float CuratorIntroScaleTime = 0.0f;

        float MeditationTime = 0.0f;
        const float RequiredMeditationTime = 10.0f;
        bool bObjectCreated = false;
        float CreationHoldTime = 0.0f;
        const float RequiredCreationHoldTime = 3.0f;
        float UnityProgress = 0.0f;
        const float RequiredUnity = 100.0f;

        UPROPERTY()
        AActor* RealityObject;
};
