#include "MoodSystem.h"
#include "Engine/Engine.h"

UMoodSystem::UMoodSystem()
{
}

void UMoodSystem::Initialize()
{
    if (bIsInitialized) return;

    Calm = 0.0f;
    Love = 0.0f;
    Joy = 0.0f;
    Fear = 0.0f;
    Anger = 0.0f;
    Sadness = 0.0f;
    Stress = 0.0f;

    bIsInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("UMoodSystem initialized"));
}

void UMoodSystem::Update(float DeltaTime)
{
    if (!bIsInitialized) return;

    float DT = DeltaTime / 60.0f;

    Calm = FMath::Max(0.0f, Calm - PositiveDecay * DT);
    Love = FMath::Max(0.0f, Love - PositiveDecay * DT);
    Joy = FMath::Max(0.0f, Joy - PositiveDecay * DT);

    Fear = FMath::Max(0.0f, Fear - NegativeDecay * DT);
    Anger = FMath::Max(0.0f, Anger - NegativeDecay * DT);
    Sadness = FMath::Max(0.0f, Sadness - NegativeDecay * DT);
    Stress = FMath::Max(0.0f, Stress - NegativeDecay * DT);

    ClampAllEmotions();
}

void UMoodSystem::ClampAllEmotions()
{
    Calm = FMath::Clamp(Calm, 0.0f, 1.0f);
    Love = FMath::Clamp(Love, 0.0f, 1.0f);
    Joy = FMath::Clamp(Joy, 0.0f, 1.0f);
    Fear = FMath::Clamp(Fear, 0.0f, 1.0f);
    Anger = FMath::Clamp(Anger, 0.0f, 1.0f);
    Sadness = FMath::Clamp(Sadness, 0.0f, 1.0f);
    Stress = FMath::Clamp(Stress, 0.0f, 1.0f);
}

void UMoodSystem::AddCalm(float Value)
{
    Calm = FMath::Clamp(Calm + Value, 0.0f, 1.0f);
    OnEmotionChanged.Broadcast(EmotionType::Calm, Calm);
}

void UMoodSystem::AddLove(float Value)
{
    Love = FMath::Clamp(Love + Value, 0.0f, 1.0f);
    OnEmotionChanged.Broadcast(EmotionType::Love, Love);
}

void UMoodSystem::AddJoy(float Value)
{
    Joy = FMath::Clamp(Joy + Value, 0.0f, 1.0f);
    OnEmotionChanged.Broadcast(EmotionType::Happy, Joy);
}

void UMoodSystem::AddFear(float Value)
{
    Fear = FMath::Clamp(Fear + Value, 0.0f, 1.0f);
    OnEmotionChanged.Broadcast(EmotionType::Anxious, Fear);
}

void UMoodSystem::AddAnger(float Value)
{
    Anger = FMath::Clamp(Anger + Value, 0.0f, 1.0f);
    OnEmotionChanged.Broadcast(EmotionType::Angry, Anger);
}

void UMoodSystem::AddSadness(float Value)
{
    Sadness = FMath::Clamp(Sadness + Value, 0.0f, 1.0f);
    OnEmotionChanged.Broadcast(EmotionType::Sad, Sadness);
}

void UMoodSystem::AddStress(float Value)
{
    Stress = FMath::Clamp(Stress + Value, 0.0f, 1.0f);
    OnEmotionChanged.Broadcast(EmotionType::Stressed, Stress);
}

float UMoodSystem::GetWellbeingIndex() const
{
    float Positives = (Calm + Love + Joy) / 3.0f;
    float Negatives = (Fear + Anger + Sadness + Stress) / 4.0f;
    float Raw = Positives - Negatives;
    Raw += (Resilience - 0.5f) * 0.3f;
    return FMath::Clamp(Raw, -1.0f, 1.0f);
}

FMoodState UMoodSystem::GetCurrentMoodState() const
{
    float Emotions[7] = { Calm, Love, Joy, Fear, Anger, Sadness, Stress };
    int32 MaxIndex = 0;
    float MaxValue = Emotions[0];

    for (int32 i = 1; i < 7; i++)
    {
        if (Emotions[i] > MaxValue)
        {
            MaxValue = Emotions[i];
            MaxIndex = i;
        }
    }

    EmotionType Type = static_cast<EmotionType>(MaxIndex + 1);

    if (MaxValue < 0.1f)
    {
        Type = EmotionType::Neutral;
        MaxValue = 0.0f;
    }

    return FMoodState(Type, MaxValue);
}

bool UMoodSystem::HasActiveMoodEffects() const
{
    return Calm > 0.2f || Love > 0.2f || Joy > 0.2f || Fear > 0.2f || Anger > 0.2f || Sadness > 0.2f || Stress > 0.2f;
}

TArray<FString> UMoodSystem::GetActiveMoodEffects() const
{
    TArray<FString> Effects;

    if (Calm > 0.2f) Effects.Add(FString::Printf(TEXT("Calm: %.2f"), Calm));
    if (Love > 0.2f) Effects.Add(FString::Printf(TEXT("Love: %.2f"), Love));
    if (Joy > 0.2f) Effects.Add(FString::Printf(TEXT("Joy: %.2f"), Joy));
    if (Fear > 0.2f) Effects.Add(FString::Printf(TEXT("Fear: %.2f"), Fear));
    if (Anger > 0.2f) Effects.Add(FString::Printf(TEXT("Anger: %.2f"), Anger));
    if (Sadness > 0.2f) Effects.Add(FString::Printf(TEXT("Sadness: %.2f"), Sadness));
    if (Stress > 0.2f) Effects.Add(FString::Printf(TEXT("Stress: %.2f"), Stress));

    return Effects;
}

void UMoodSystem::ApplyMeditationBoost()
{
    AddCalm(0.2f);
    AddLove(0.1f);
    AddJoy(0.05f);
    AddStress(-0.2f);
    AddFear(-0.1f);

    UE_LOG(LogTemp, Log, TEXT("Meditation boost applied"));
}

void UMoodSystem::ApplyHelpOthersBoost()
{
    AddLove(0.15f);
    AddJoy(0.1f);
    AddStress(-0.05f);
    Empathy = FMath::Clamp(Empathy + 0.01f, 0.0f, 1.0f);

    UE_LOG(LogTemp, Log, TEXT("Help others boost applied"));
}

void UMoodSystem::ApplyDamageShock(float Damage)
{
    float Factor = FMath::Clamp(Damage / 100.0f, 0.0f, 1.0f);

    AddStress(0.2f * Factor);
    AddFear(0.15f * Factor);
    AddCalm(-0.1f * Factor);

    UE_LOG(LogTemp, Log, TEXT("Damage shock applied: %.1f"), Damage);
}

void UMoodSystem::ApplyStagnationPenalty()
{
    AddJoy(-0.1f);
    AddLove(-0.1f);
    AddSadness(0.1f);
    AddStress(0.1f);

    UE_LOG(LogTemp, Log, TEXT("Stagnation penalty applied"));
}
