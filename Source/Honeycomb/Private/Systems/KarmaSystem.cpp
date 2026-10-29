#include "KarmaSystem.h"
#include "Engine/Engine.h"

UKarmaSystem::UKarmaSystem()
{
    CurrentKarma = 0.0f;
    MaxKarma = 1000.0f;
    MinKarma = -1000.0f;
    KarmaHistory.Empty();
    MaxHistorySize = 100;
    GameManager = nullptr;
    PlayerController = nullptr;

    UE_LOG(LogTemp, Verbose, TEXT("UKarmaSystem: Constructor initialized"));
}

void UKarmaSystem::Initialize()
{
    if (bIsInitialized) return;
    CurrentKarma = 0.0f;
    PreviousTier = KarmaTier::Neutral;
    bIsInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("UKarmaSystem initialized"));
}

void UKarmaSystem::Update(float DeltaTime)
{
    if (!bIsInitialized) return;

    if (PassiveDecayPerMinute > 0.0f && FMath::Abs(CurrentKarma) > 0.1f)
    {
        float DecayAmount = PassiveDecayPerMinute / 60.0f * DeltaTime;
        if (CurrentKarma > 0)
        {
            CurrentKarma = FMath::Max(0.0f, CurrentKarma - DecayAmount);
        }
        else
        {
            CurrentKarma = FMath::Min(0.0f, CurrentKarma + DecayAmount);
        }
    }
}

void UKarmaSystem::AddKarma(float Amount)
{
    CurrentKarma = FMath::Clamp(CurrentKarma + Amount, MinKarma, MaxKarma);

    KarmaTier NewTier = GetKarmaTier();
    if (NewTier != PreviousTier)
    {
        PreviousTier = NewTier;
        OnKarmaTierChanged.Broadcast(NewTier);
        UE_LOG(LogTemp, Log, TEXT("Karma tier changed to: %s"), *GetKarmaTierName());
    }

    OnKarmaChanged.Broadcast(CurrentKarma);
}

void UKarmaSystem::AddKarmaForAction(KarmaAction Action)
{
    float Amount = 0.0f;

    switch (Action)
    {
        case KarmaAction::Meditation:
            Amount = 2.0f;
            break;
        case KarmaAction::DeepMeditation:
            Amount = 10.0f;
            break;
        case KarmaAction::HealSelf:
            Amount = 1.0f;
            break;
        case KarmaAction::HealOthers:
            Amount = 5.0f;
            break;
        case KarmaAction::HelpNPC:
            Amount = 3.0f;
            break;
        case KarmaAction::HarmNPC:
            Amount = -6.0f;
            break;
        case KarmaAction::Truth:
            Amount = 2.0f;
            break;
        case KarmaAction::Lie:
            Amount = -2.0f;
            break;
        default:
            return;
    }

    AddKarma(Amount);
    UE_LOG(LogTemp, Verbose, TEXT("Karma: %s -> %.1f"), *UEnum::GetValueAsString(Action), Amount);
}

KarmaTier UKarmaSystem::GetKarmaTier() const
{
    if (CurrentKarma >= 400.0f) {
        return KarmaTier::Radiant;
    }

    if (CurrentKarma >= 100.0f) {
        return KarmaTier::Light;
    }

    if (CurrentKarma <= -400.0f) {
        return KarmaTier::Dark;
    }
    
    if (CurrentKarma <= -100.0f) {
        return KarmaTier::Shadow;
    }

    return KarmaTier::Neutral;
}

float UKarmaSystem::GetKarmaPercentage() const
{
    return (CurrentKarma - MinKarma) / (MaxKarma - MinKarma);
}

FString UKarmaSystem::GetKarmaTierName() const
{
    switch (GetKarmaTier())
    {
        case KarmaTier::Radiant:
            return TEXT("Сияющий");
        case KarmaTier::Light:
            return TEXT("Светлый");
        case KarmaTier::Neutral:
            return TEXT("Нейтральный");
        case KarmaTier::Shadow:
            return TEXT("Теневой");
        case KarmaTier::Dark:
            return TEXT("Тёмный");
        default:
            return TEXT("Неизвестно");
    }
}

float UKarmaSystem::GetKarmaMultiplier() const
{
    switch (GetKarmaTier())
    {
        case KarmaTier::Radiant:
            return 2.0f;
        case KarmaTier::Light:
            return 1.5f;
        case KarmaTier::Neutral:
            return 1.0f;
        case KarmaTier::Shadow:
            return 0.8f;
        case KarmaTier::Dark:
            return 0.5f;
        default:
            return 1.0f;
    }
}
