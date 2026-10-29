#pragma once

#include "HoneycombCore.h"
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "BirthProfileSystem.generated.h"

USTRUCT(BlueprintType)
struct FBirthProfile
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FDateTime BirthDateUtc;

    UPROPERTY(BlueprintReadWrite)
    FString BirthPlace;

    UPROPERTY(BlueprintReadWrite)
    int32 SoulMatrixSeed;

    UPROPERTY(BlueprintReadWrite)
    int32 LifePath;

    FBirthProfile(): SoulMatrixSeed(0), LifePath(1) {}
};

UCLASS(BlueprintType, Blueprintable)
class HONEYCOMB_API UBirthProfileSystem: public UObject
{
    GENERATED_BODY()

    public:
        UBirthProfileSystem();

        UFUNCTION(BlueprintCallable, Category = "FBirthProfile")
        void Initialize(FDateTime BirthDateUtc, const FString& PlaceName);

        UFUNCTION(BlueprintCallable, Category = "FBirthProfile")
        FBirthProfile GetProfile() const { return Profile; }

        UFUNCTION(BlueprintCallable, Category = "FBirthProfile")
        float GetInitialEnergyBonus() const { return InitialEnergyBonus; }

        UFUNCTION(BlueprintCallable, Category = "FBirthProfile")
        int32 GetInitialChakraBiasIndex() const { return InitialChakraBiasIndex; }

        UFUNCTION(BlueprintCallable, Category = "FBirthProfile")
        float GetCosmoChannelAffinity() const { return CosmoChannelAffinity; }

    private:
        int32 ComputeSoulMatrixSeed(FDateTime Date, const FString& Place);
        int32 ComputeLifePathNumber(FDateTime Date);
        void DeriveModifiers();

        UPROPERTY(BlueprintReadWrite, Category = "FBirthProfile", meta = (AllowPrivateAccess = "true"))
        FBirthProfile Profile;

        UPROPERTY(BlueprintReadWrite, Category = "FBirthProfile|Derived", meta = (AllowPrivateAccess = "true"))
        float InitialEnergyBonus = 0.0f;

        UPROPERTY(BlueprintReadWrite, Category = "FBirthProfile|Derived", meta = (AllowPrivateAccess = "true"))
        int32 InitialChakraBiasIndex = 0;

        UPROPERTY(BlueprintReadWrite, Category = "FBirthProfile|Derived", meta = (AllowPrivateAccess = "true"))
        float CosmoChannelAffinity = 0.0f;

        UPROPERTY(BlueprintReadWrite, Category = "FBirthProfile|Raw", meta = (AllowPrivateAccess = "true"))
        FDateTime BirthDate;

        UPROPERTY(BlueprintReadWrite, Category = "FBirthProfile|Raw", meta = (AllowPrivateAccess = "true"))
        FString BirthPlace;

        UPROPERTY(BlueprintReadWrite, Category = "FBirthProfile|Raw", meta = (AllowPrivateAccess = "true"))
        FString BirthTime;

        UPROPERTY(BlueprintReadWrite, Category = "FBirthProfile|Raw", meta = (AllowPrivateAccess = "true"))
        int32 BirthDayOfWeek = 0;

        UPROPERTY(BlueprintReadWrite, Category = "FBirthProfile|Raw", meta = (AllowPrivateAccess = "true"))
        FString AstrologyChart;

        UPROPERTY(BlueprintReadOnly, Category = "FBirthProfile|Raw", meta = (AllowPrivateAccess = "true"))
        bool bIsInitialized = false;
};
