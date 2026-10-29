#pragma once

#include "HoneycombCore.h"
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DiseaseCatalog.generated.h"

USTRUCT(BlueprintType)
struct FDiseaseEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString Id;

    UPROPERTY(BlueprintReadWrite)
    FString Name;

    UPROPERTY(BlueprintReadWrite)
    FString Type;

    UPROPERTY(BlueprintReadWrite)
    FString Description;

    UPROPERTY(BlueprintReadWrite)
    TArray<FString> Symptoms;
};

UCLASS(BlueprintType, Blueprintable)
class HONEYCOMB_API UDiseaseCatalog: public UObject
{
    GENERATED_BODY()

    public:
        UDiseaseCatalog();

        UFUNCTION(BlueprintCallable, Category = "FDisease")
        void Load();

        UFUNCTION(BlueprintCallable, Category = "FDisease")
        FDiseaseEntry FindById(const FString& Id) const;

        UFUNCTION(BlueprintCallable, Category = "FDisease")
        TArray<FDiseaseEntry> FindByType(const FString& Type) const;

        UFUNCTION(BlueprintCallable, Category = "FDisease")
        TArray<FDiseaseEntry> GetAllEntries() const {
            return Entries;
        }

        UFUNCTION(BlueprintCallable, Category = "FDisease")
        int32 GetEntryCount() const {
            return Entries.Num();
        }

        void SetJsonData(const FString& JsonString);

    private:
        UPROPERTY()
        TArray<FDiseaseEntry> Entries;

        UPROPERTY()
        TArray<FDiseaseEntry> Diseases;

        UPROPERTY()
        TMap<FString, int32> DiseaseIndexById;

        UPROPERTY()
        FString CachedJson;

        UPROPERTY()
        bool bIsInitialized = false;
};
