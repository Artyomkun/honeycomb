#pragma once

#include "HoneycombCore.h"
#include "CoreMinimal.h"
#include "KarmaSystem.h"
#include "UObject/Object.h"
#include "ReincarnationSystem.generated.h"

UCLASS(BlueprintType, Blueprintable)
class HONEYCOMB_API UReincarnationSystem: public UObject
{
    GENERATED_BODY()

    public:
        UReincarnationSystem();

        void Initialize();
        void Update(float DeltaTime);

        UFUNCTION(BlueprintCallable, Category = "Reincarnation")
        void Reincarnate();

        UFUNCTION(BlueprintCallable, Category = "Reincarnation")
        int32 GetReincarnationCount() const {
            return ReincarnationCount;
        }

        UFUNCTION(BlueprintCallable, Category = "Reincarnation")
        FString GetLastKarmaTier() const {
            return LastKarmaTier;
        }

        UFUNCTION(BlueprintCallable, Category = "Reincarnation")
        bool CanReincarnate() const {
            return bCanReincarnate;
        }

        UFUNCTION(BlueprintCallable, Category = "Reincarnation")
        void SetCanReincarnate(bool bCan) {
            bCanReincarnate = bCan;
        }

        void SetGameManager(class AGameManager* GM) {
            GameManagerRef = GM;
        }

        void SetWorldSystem(class UWorldSystem* System) {
            WorldSystemRef = System;
        }

        void SetKarmaSystem(class UKarmaSystem* System) {
            KarmaSystemRef = System;
        }

        void SetChakraSystem(class UChakraSystem* System) {
            ChakraSystemRef = System;
        }

        void SetDimensionSystem(class UDimensionSystem* System) {
            DimensionSystemRef = System;
        }

        DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReincarnation, int32, NewCount);
        DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnReincarnationWithKarma, int32, Count, FString, KarmaTier);

        UPROPERTY(BlueprintAssignable, Category = "Reincarnation|Events")
        FOnReincarnation OnReincarnation;

        UPROPERTY(BlueprintAssignable, Category = "Reincarnation|Events")
        FOnReincarnationWithKarma OnReincarnationWithKarma;

    private:
        void ApplyKarmaEffectsOnRebirth(KarmaTier Tier);

        UPROPERTY(BlueprintReadWrite, Category = "Reincarnation", meta = (AllowPrivateAccess = "true"))
        int32 ReincarnationCount = 0;

        UPROPERTY(BlueprintReadWrite, Category = "Reincarnation", meta = (AllowPrivateAccess = "true"))
        FString LastKarmaTier = TEXT("Neutral");

        UPROPERTY(BlueprintReadWrite, Category = "Reincarnation", meta = (AllowPrivateAccess = "true"))
        bool bCanReincarnate = true;

        UPROPERTY()
        class AGameManager* GameManagerRef = nullptr;

        UPROPERTY()
        class UWorldSystem* WorldSystemRef = nullptr;

        UPROPERTY()
        class UKarmaSystem* KarmaSystemRef = nullptr;

        UPROPERTY()
        class UChakraSystem* ChakraSystemRef = nullptr;

        UPROPERTY()
        class UDimensionSystem* DimensionSystemRef = nullptr;

        bool bIsInitialized = false;
};
