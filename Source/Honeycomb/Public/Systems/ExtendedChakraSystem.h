#pragma once

#include "HoneycombCore.h"
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ExtendedChakraSystem.generated.h"

USTRUCT(BlueprintType)
struct FExtendedChakra
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    int32 Index = 0;

    UPROPERTY(BlueprintReadWrite)
    FString Name;

    UPROPERTY(BlueprintReadWrite)
    FString Mantra;

    UPROPERTY(BlueprintReadWrite)
    TArray<FString> Abilities;

    UPROPERTY(BlueprintReadWrite)
    float EnergyMultiplier = 1.0f;

    UPROPERTY(BlueprintReadWrite)
    bool bUnlocked = false;
};

UCLASS(BlueprintType, Blueprintable)
class HONEYCOMB_API UExtendedChakraSystem: public UObject
{
    GENERATED_BODY()

    public:
        UExtendedChakraSystem();

        UFUNCTION(BlueprintCallable, Category = "FExtendedChakra")
        void Initialize154();

        UFUNCTION(BlueprintCallable, Category = "FExtendedChakra")
        FExtendedChakra GetChakraByIndex(int32 Index) const;

        UFUNCTION(BlueprintCallable, Category = "FExtendedChakra")
        TArray<FExtendedChakra> GetAllChakras() const {
            return Chakras;
        }

        UFUNCTION(BlueprintCallable, Category = "FExtendedChakra")
        int32 GetCurrentChakraIndex() const {
            return CurrentChakraIndex;
        }

        UFUNCTION(BlueprintCallable, Category = "FExtendedChakra")
        void SetCurrentChakraIndex(int32 NewIndex);

        UFUNCTION(BlueprintCallable, Category = "FExtendedChakra")
        void UnlockChakra(int32 Index);

        UFUNCTION(BlueprintCallable, Category = "FExtendedChakra")
        bool IsChakraUnlocked(int32 Index) const;

        UFUNCTION(BlueprintCallable, Category = "FExtendedChakra")
        int32 GetUnlockedCount() const;

        void SetBirthProfile(class UBirthProfileSystem* Profile) {
            FBirthProfile = Profile;
        }
        void SetCosmoenergySystem(class UCosmoenergySystem* System) {
            CosmoSystem = System;
        }

    private:
        UPROPERTY()
        TArray<FExtendedChakra> Chakras;

        UPROPERTY()
        int32 CurrentChakraIndex = 0;

        UPROPERTY()
        TArray<int32> ChakraLevels;

        UPROPERTY()
        TArray<float> ChakraEnergies;

        UPROPERTY()
        float TotalEnergy = 0.0f;

        UPROPERTY()
        bool bKundaliniAwakened = false;

        UPROPERTY()
        class UChakraSystem* BaseChakraSystem = nullptr;

        UPROPERTY()
        class UBirthProfileSystem* FBirthProfile = nullptr;

        UPROPERTY()
        class UCosmoenergySystem* CosmoSystem = nullptr;

        bool bIsInitialized = false;
};
