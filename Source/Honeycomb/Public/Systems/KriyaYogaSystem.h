#pragma once

#include "HoneycombCore.h"
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "KriyaYogaSystem.generated.h"

USTRUCT(BlueprintType)
struct FKriyaTechnique
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString Name;

    UPROPERTY(BlueprintReadWrite)
    FString Mantra;

    UPROPERTY(BlueprintReadWrite)
    FString Description;

    UPROPERTY(BlueprintReadWrite)
    float EnergyMultiplier = 1.0f;

    UPROPERTY(BlueprintReadWrite)
    FString Effect;

    FKriyaTechnique() {}

    FKriyaTechnique(const FString& InName, const FString& InMantra, const FString& InDescription, float InEnergyMultiplier, const FString& InEffect): Name(InName), Mantra(InMantra), Description(InDescription), EnergyMultiplier(InEnergyMultiplier), Effect(InEffect) {}
};

UCLASS(BlueprintType, Blueprintable)
class HONEYCOMB_API UKriyaYogaSystem: public UObject
{
    GENERATED_BODY()

    public:
        UKriyaYogaSystem();

        void Initialize();
        void Update(float DeltaTime);

        DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKriyaComplete, int32, KriyaLevel);

        UPROPERTY(BlueprintAssignable, Category = "Kriya|Events")
        FOnKriyaComplete OnKriyaComplete;

        UFUNCTION(BlueprintCallable, Category = "Kriya")
        void StartKriyaPractice(int32 KriyaLevel);

        UFUNCTION(BlueprintCallable, Category = "Kriya")
        void StopKriyaPractice();

        UFUNCTION(BlueprintCallable, Category = "Kriya")
        bool IsKriyaActive() const {
            return bIsKriyaActive;
        }

        UFUNCTION(BlueprintCallable, Category = "Kriya")
        float GetKriyaProgress() const;

        UFUNCTION(BlueprintCallable, Category = "Kriya")
        float GetKundaliniEnergy() const {
            return KundaliniEnergy;
        }

        UFUNCTION(BlueprintCallable, Category = "Kriya")
        float GetPranaEnergy() const {
            return PranaEnergy;
        }

        UFUNCTION(BlueprintCallable, Category = "Kriya")
        bool IsChakraActivated(int32 ChakraLevel) const;

        UFUNCTION(BlueprintCallable, Category = "Kriya")
        FString GetCurrentTechniqueName() const;

        UFUNCTION(BlueprintCallable, Category = "Kriya")
        int32 GetCurrentTechniqueLevel() const {
            return CurrentKriyaLevel;
        }

        UFUNCTION(BlueprintCallable, Category = "Kriya")
        bool IsKriyaUnlocked(int32 KriyaLevel) const;

        UFUNCTION(BlueprintCallable, Category = "Kriya")
        float GetChakraEnergyLevel(int32 Index) const;

        void SetChakraSystem(class UChakraSystem* System) { 
            ChakraSystemRef = System;
        }

        void SetGameManager(class AGameManager* GM) {
            GameManagerRef = GM;
        }

    private:
        void InitializeKriyaTechniques();
        void UpdateKriyaPractice(float DeltaTime);
        float CalculateKriyaMultiplier() const;
        bool CanPracticeKriya(int32 KriyaLevel) const;
        bool HasMasteredChakras(int32 FromLevel, int32 ToLevel) const;
        void ActivateChakra(int32 ChakraLevel);
        void ApplyKriyaEffects(bool bIsStarting);
        void UpdatePranaFlow(float DeltaTime);
        void UpdateKundaliniFlow(float DeltaTime);
        void AwakenKundalini();
        float GetRequiredKriyaTime(int32 KriyaLevel) const;
        void CheckKriyaProgress();
        void CompleteKriyaPractice();
        void CheckChakraAdvancement();

        UPROPERTY()
        class UChakraSystem* ChakraSystemRef = nullptr;

        UPROPERTY()
        class AGameManager* GameManagerRef = nullptr;

        UPROPERTY()
        uint8 ChakraActivated[12];

        UPROPERTY()
        float ChakraEnergyLevels[12];

        UPROPERTY()
        float KundaliniFlow[12];

        UPROPERTY()
        bool bIsKriyaActive = false;

        UPROPERTY()
        float CurrentKriyaTime = 0.0f;

        UPROPERTY()
        int32 CurrentKriyaLevel = 1;

        UPROPERTY()
        float KundaliniEnergy = 0.0f;

        UPROPERTY()
        float PranaEnergy = 0.0f;

        UPROPERTY()
        float MaxChakraEnergy = 1000.0f;

        UPROPERTY()
        float KriyaEnergyMultiplier = 2.0f;

        UPROPERTY()
        float KundaliniAwakeningThreshold = 1000.0f;

        UPROPERTY()
        float PranaFlowRate = 10.0f;

        UPROPERTY()
        TMap<int32, FKriyaTechnique> KriyaTechniques;

        bool bIsInitialized = false;
        bool bKundaliniAwakened = false;
};
