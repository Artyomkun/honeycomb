#pragma once

#include "HoneycombCore.h"
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DiseaseSystem.generated.h"

UENUM(BlueprintType)
enum class DiseaseType: uint8
{
    Physical,
    Mental,
    Psychological,
    Spiritual,
    Astral
};

USTRUCT(BlueprintType)
struct FDisease
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString Id;

    UPROPERTY(BlueprintReadWrite)
    FString Name;

    UPROPERTY(BlueprintReadWrite)
    DiseaseType Type = DiseaseType::Physical;

    UPROPERTY(BlueprintReadWrite)
    float Severity = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    FString Description;

    UPROPERTY(BlueprintReadWrite)
    TArray<FString> Symptoms;

    UPROPERTY(BlueprintReadWrite)
    bool bIsActive = false;
};

UENUM(BlueprintType)
enum class DiseaseKarmaTier: uint8
{
    Radiant,
    Light,
    Shadow,
    Dark
};

UCLASS(BlueprintType, Blueprintable)
class HONEYCOMB_API UDiseaseSystem: public UObject
{
    GENERATED_BODY()

    public:
        UDiseaseSystem();

        UFUNCTION(BlueprintCallable, Category = "FDisease")
        void Initialize();

        UFUNCTION(BlueprintCallable, Category = "FDisease")
        void Update(float DeltaTime);

        UFUNCTION(BlueprintCallable, Category = "FDisease")
        void AddDisease(const FDisease& NewDisease);

        UFUNCTION(BlueprintCallable, Category = "FDisease")
        void CureDisease(const FString& Id, float Amount);

        UFUNCTION(BlueprintCallable, Category = "FDisease")
        float GetTotalSeverity() const;

        UFUNCTION(BlueprintCallable, Category = "FDisease")
        TArray<FDisease> GetActiveDiseases() const;

        UFUNCTION(BlueprintCallable, Category = "FDisease")
        bool HasActiveDiseases() const;

        UFUNCTION(BlueprintCallable, Category = "FDisease")
        float GetBodyTemperature() const;

        void SetMoodSystem(class UMoodSystem* System) {
            MoodSystemRef = System;
        }

        void SetChakraSystem(class UChakraSystem* System) {
            ChakraSystemRef = System;
        }

        void SetKarmaSystem(class UKarmaSystem* System) {
            KarmaSystemRef = System;
        }
        
        void SetDiseaseCatalog(class UDiseaseCatalog* Catalog) {
            DiseaseCatalogRef = Catalog;
        }

    private:
        float GetProgressDelta(const FDisease& D);
        float GetDiscipline() const;
        float GetChakraBalanceFactor() const;

        UPROPERTY()
        TArray<FDisease> ActiveDiseases;

        UPROPERTY()
        class UMoodSystem* MoodSystemRef = nullptr;

        UPROPERTY()
        class UChakraSystem* ChakraSystemRef = nullptr;

        UPROPERTY()
        class UKarmaSystem* KarmaSystemRef = nullptr;

        UPROPERTY()
        class UDiseaseCatalog* DiseaseCatalogRef = nullptr;

        bool bIsInitialized = false;
};
