#pragma once

#include "HoneycombCore.h"
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CosmoenergySystem.generated.h"

USTRUCT(BlueprintType)
struct FCosmoChannel
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString Id;

    UPROPERTY(BlueprintReadWrite)
    FString Name;

    UPROPERTY(BlueprintReadWrite)
    float Resonance = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    TArray<int32> ChakraIndices;
};

UCLASS(BlueprintType, Blueprintable)
class HONEYCOMB_API UCosmoenergySystem: public UObject
{
    GENERATED_BODY()

    public:
        UCosmoenergySystem();

        UFUNCTION(BlueprintCallable, Category = "Cosmoenergy")
        void InitializeChannels();

        UFUNCTION(BlueprintCallable, Category = "Cosmoenergy")
        void ComputeGlobalResonance();

        UFUNCTION(BlueprintCallable, Category = "Cosmoenergy")
        float GetChannelResonance(const FString& ChannelId) const;

        UFUNCTION(BlueprintCallable, Category = "Cosmoenergy")
        float GetGlobalResonance() const { return GlobalResonance; }

        UFUNCTION(BlueprintCallable, Category = "Cosmoenergy")
        TArray<FCosmoChannel> GetAllChannels() const { return Channels; }

        UFUNCTION(BlueprintCallable, Category = "Cosmoenergy")
        void SetBirthProfileSystem(class UBirthProfileSystem* Profile) { FBirthProfile = Profile; }

    private:
        FCosmoChannel CreateChannel(const FString& Id, const FString& Name, const TArray<int32>& ChakraIdx);

        UPROPERTY(BlueprintReadWrite, Category = "Cosmoenergy", meta = (AllowPrivateAccess = "true"))
        TArray<FCosmoChannel> Channels;

        UPROPERTY(BlueprintReadWrite, Category = "Cosmoenergy", meta = (AllowPrivateAccess = "true"))
        float GlobalResonance = 0.5f;

        UPROPERTY()
        class UBirthProfileSystem* FBirthProfile = nullptr;

        bool bIsInitialized = false;
};
