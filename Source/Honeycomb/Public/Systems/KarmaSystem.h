#pragma once

#include "HoneycombCore.h"
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "KarmaSystem.generated.h"

UENUM(BlueprintType)
enum class KarmaTier: uint8
{
    Dark = 0,
    Shadow = 1,
    Neutral = 2,
    Light = 3,
    Radiant = 4
};

UENUM(BlueprintType)
enum class KarmaAction: uint8
{
    Meditation,
    HealSelf,
    HealOthers,
    HelpNPC,
    HarmNPC,
    Truth,
    Lie,
    DeepMeditation
};

UCLASS(BlueprintType, Blueprintable)
class HONEYCOMB_API UKarmaSystem: public UObject
{
    GENERATED_BODY()

    public:
        UKarmaSystem();

        UFUNCTION(BlueprintCallable, Category = "Karma")
        void Initialize();

        UFUNCTION(BlueprintCallable, Category = "Karma")
        void Update(float DeltaTime);

        UFUNCTION(BlueprintCallable, Category = "Karma")
        void AddKarma(float Amount);

        UFUNCTION(BlueprintCallable, Category = "Karma")
        void AddKarmaForAction(KarmaAction Action);

        UFUNCTION(BlueprintCallable, Category = "Karma")
        KarmaTier GetKarmaTier() const;

        UFUNCTION(BlueprintCallable, Category = "Karma")
        float GetKarmaValue() const {
            return CurrentKarma;
        }

        UFUNCTION(BlueprintCallable, Category = "Karma")
        float GetKarmaPercentage() const;

        UFUNCTION(BlueprintCallable, Category = "Karma")
        FString GetKarmaTierName() const;

        UFUNCTION(BlueprintCallable, Category = "Karma")
        void SetPassiveDecay(float DecayPerMinute) {
            PassiveDecayPerMinute = DecayPerMinute;
        }

        UFUNCTION(BlueprintCallable, Category = "Karma")
        float GetKarmaMultiplier() const;

        DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKarmaChanged, float, NewKarma);
        DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKarmaTierChanged, KarmaTier, NewTier);

        UPROPERTY(BlueprintAssignable, Category = "Karma|Events")
        FOnKarmaChanged OnKarmaChanged;

        UPROPERTY(BlueprintAssignable, Category = "Karma|Events")
        FOnKarmaTierChanged OnKarmaTierChanged;

    private:
        UPROPERTY(BlueprintReadWrite, Category = "Karma", meta = (AllowPrivateAccess = "true"))
        float CurrentKarma = 0.0f;

        UPROPERTY(BlueprintReadWrite, Category = "Karma", meta = (AllowPrivateAccess = "true"))
        float MinKarma = -1000.0f;

        UPROPERTY(BlueprintReadWrite, Category = "Karma", meta = (AllowPrivateAccess = "true"))
        float MaxKarma = 1000.0f;

        UPROPERTY(BlueprintReadWrite, Category = "Karma", meta = (AllowPrivateAccess = "true"))
        float PassiveDecayPerMinute = 0.0f;

        UPROPERTY(BlueprintReadWrite, Category = "Karma|State", meta = (AllowPrivateAccess = "true"))
        TArray<FString> KarmaHistory;

        UPROPERTY(BlueprintReadWrite, Category = "Karma|State", meta = (AllowPrivateAccess = "true"))
        int32 MaxHistorySize = 100;

        UPROPERTY(BlueprintReadOnly, Category = "Karma|References", meta = (AllowPrivateAccess = "true"))
        class AGameManager* GameManager = nullptr;

        UPROPERTY(BlueprintReadOnly, Category = "Karma|References", meta = (AllowPrivateAccess = "true"))
        class AVRPlayerController* PlayerController = nullptr;

        KarmaTier PreviousTier = KarmaTier::Neutral;
        bool bIsInitialized = false;
};
