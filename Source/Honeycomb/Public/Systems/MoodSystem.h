#pragma once

#include "HoneycombCore.h"
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MoodSystem.generated.h"

UENUM(BlueprintType)
enum class EmotionType: uint8
{
    Neutral = 0,
    Calm = 1,
    Love = 2,
    Happy = 3,
    Anxious = 4,
    Angry = 5,
    Sad = 6,
    Stressed = 7
};

USTRUCT(BlueprintType)
struct FMoodState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    EmotionType CurrentEmotion = EmotionType::Neutral;

    UPROPERTY(BlueprintReadWrite)
    float Intensity = 0.5f;

    FMoodState() {}

    FMoodState(EmotionType InEmotion, float InIntensity): CurrentEmotion(InEmotion), Intensity(InIntensity) {}
};

UCLASS(BlueprintType, Blueprintable)
class HONEYCOMB_API UMoodSystem: public UObject
{
    GENERATED_BODY()

    public:
        UMoodSystem();

        void Initialize();
        void Update(float DeltaTime);

        UFUNCTION(BlueprintCallable, Category = "Mood")
        void AddCalm(float Value);

        UFUNCTION(BlueprintCallable, Category = "Mood")
        void AddLove(float Value);

        UFUNCTION(BlueprintCallable, Category = "Mood")
        void AddJoy(float Value);

        UFUNCTION(BlueprintCallable, Category = "Mood")
        void AddFear(float Value);

        UFUNCTION(BlueprintCallable, Category = "Mood")
        void AddAnger(float Value);

        UFUNCTION(BlueprintCallable, Category = "Mood")
        void AddSadness(float Value);

        UFUNCTION(BlueprintCallable, Category = "Mood")
        void AddStress(float Value);

        UFUNCTION(BlueprintCallable, Category = "Mood")
        float GetCalm() const {
            return Calm;
        }

        UFUNCTION(BlueprintCallable, Category = "Mood")
        float GetLove() const {
            return Love;
        }

        UFUNCTION(BlueprintCallable, Category = "Mood")
        float GetJoy() const {
            return Joy;
        }

        UFUNCTION(BlueprintCallable, Category = "Mood")
        float GetFear() const {
            return Fear;
        }

        UFUNCTION(BlueprintCallable, Category = "Mood")
        float GetAnger() const {
            return Anger;
        }

        UFUNCTION(BlueprintCallable, Category = "Mood")
        float GetSadness() const {
            return Sadness;
        }

        UFUNCTION(BlueprintCallable, Category = "Mood")
        float GetStress() const {
            return Stress;
        }

        UFUNCTION(BlueprintCallable, Category = "Mood")
        float GetResilience() const {
            return Resilience;
        }

        UFUNCTION(BlueprintCallable, Category = "Mood")
        float GetEmpathy() const {
            return Empathy;
        }

        UFUNCTION(BlueprintCallable, Category = "Mood")
        float GetDiscipline() const {
            return Discipline;
        }

        UFUNCTION(BlueprintCallable, Category = "Mood")
        float GetSpirituality() const { return Spirituality; }

        UFUNCTION(BlueprintCallable, Category = "Mood")
        float GetWellbeingIndex() const;

        UFUNCTION(BlueprintCallable, Category = "Mood")
        FMoodState GetCurrentMoodState() const;

        UFUNCTION(BlueprintCallable, Category = "Mood")
        bool HasActiveMoodEffects() const;

        UFUNCTION(BlueprintCallable, Category = "Mood")
        TArray<FString> GetActiveMoodEffects() const;

        UFUNCTION(BlueprintCallable, Category = "Mood")
        void ApplyMeditationBoost();

        UFUNCTION(BlueprintCallable, Category = "Mood")
        void ApplyHelpOthersBoost();

        UFUNCTION(BlueprintCallable, Category = "Mood")
        void ApplyDamageShock(float Damage);

        UFUNCTION(BlueprintCallable, Category = "Mood")
        void ApplyStagnationPenalty();

        DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMoodChanged, FMoodState, NewMood);
        DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEmotionChanged, EmotionType, Emotion, float, Intensity);

        UPROPERTY(BlueprintAssignable, Category = "Mood|Events")
        FOnMoodChanged OnMoodChanged;

        UPROPERTY(BlueprintAssignable, Category = "Mood|Events")
        FOnEmotionChanged OnEmotionChanged;

    private:
        void ClampAllEmotions();

        UPROPERTY(BlueprintReadWrite, Category = "Mood", meta = (AllowPrivateAccess = "true"))
        float Calm = 0.0f;

        UPROPERTY(BlueprintReadWrite, Category = "Mood", meta = (AllowPrivateAccess = "true"))
        float Love = 0.0f;

        UPROPERTY(BlueprintReadWrite, Category = "Mood", meta = (AllowPrivateAccess = "true"))
        float Joy = 0.0f;

        UPROPERTY(BlueprintReadWrite, Category = "Mood", meta = (AllowPrivateAccess = "true"))
        float Fear = 0.0f;

        UPROPERTY(BlueprintReadWrite, Category = "Mood", meta = (AllowPrivateAccess = "true"))
        float Anger = 0.0f;

        UPROPERTY(BlueprintReadWrite, Category = "Mood", meta = (AllowPrivateAccess = "true"))
        float Sadness = 0.0f;

        UPROPERTY(BlueprintReadWrite, Category = "Mood", meta = (AllowPrivateAccess = "true"))
        float Stress = 0.0f;

        UPROPERTY(BlueprintReadWrite, Category = "Mood", meta = (AllowPrivateAccess = "true"))
        float Resilience = 0.5f;

        UPROPERTY(BlueprintReadWrite, Category = "Mood", meta = (AllowPrivateAccess = "true"))
        float Empathy = 0.5f;

        UPROPERTY(BlueprintReadWrite, Category = "Mood", meta = (AllowPrivateAccess = "true"))
        float Discipline = 0.5f;

        UPROPERTY(BlueprintReadWrite, Category = "Mood", meta = (AllowPrivateAccess = "true"))
        float Spirituality = 0.5f;

        UPROPERTY()
        float PositiveDecay = 0.1f;

        UPROPERTY()
        float NegativeDecay = 0.05f;

        bool bIsInitialized = false;
};
