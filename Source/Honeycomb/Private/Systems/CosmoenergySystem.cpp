#include "CosmoenergySystem.h"
#include "BirthProfileSystem.h"
#include "Engine/Engine.h"

UCosmoenergySystem::UCosmoenergySystem()
{
    GlobalResonance = 0.5f;
    bIsInitialized = false;
}

void UCosmoenergySystem::InitializeChannels()
{
    Channels.Empty();

    TArray<int32> SolIndices = { 6, 10, 33, 72 };
    Channels.Add(CreateChannel(TEXT("SOL"), TEXT("Солнечный канал"), SolIndices));

    TArray<int32> LunIndices = { 1, 4, 28, 90 };
    Channels.Add(CreateChannel(TEXT("LUN"), TEXT("Лунный канал"), LunIndices));

    TArray<int32> TerIndices = { 0, 2, 3, 15 };
    Channels.Add(CreateChannel(TEXT("TER"), TEXT("Земной канал"), TerIndices));

    TArray<int32> AkaIndices = { 100, 120, 140, 153 };
    Channels.Add(CreateChannel(TEXT("AKA"), TEXT("Канал Акаши"), AkaIndices));

    UE_LOG(LogTemp, Log, TEXT("UCosmoenergySystem: %d channels initialized"), Channels.Num());

    ComputeGlobalResonance();
}

FCosmoChannel UCosmoenergySystem::CreateChannel(const FString& Id, const FString& Name, const TArray<int32>& ChakraIdx)
{
    FCosmoChannel Channel;
    Channel.Id = Id;
    Channel.Name = Name;
    Channel.Resonance = 0.0f;
    Channel.ChakraIndices = ChakraIdx;
    return Channel;
}

void UCosmoenergySystem::ComputeGlobalResonance()
{
    if (!FBirthProfile)
    {
        GlobalResonance = 0.5f;
        UE_LOG(LogTemp, Warning, TEXT("UCosmoenergySystem: FBirthProfile is null, using default resonance 0.5"));
        return;
    }

    float Affinity = FBirthProfile->GetCosmoChannelAffinity();

    for (FCosmoChannel& Channel: Channels)
    {
        Channel.Resonance = 0.3f + Affinity * 0.7f;
    }

    float TotalResonance = 0.0f;
    for (const FCosmoChannel& Channel: Channels)
    {
        TotalResonance += Channel.Resonance;
    }

    GlobalResonance = Channels.Num() > 0 ? TotalResonance / Channels.Num() : 0.5f;

    UE_LOG(LogTemp, Log, TEXT("UCosmoenergySystem: GlobalResonance=%.2f, Affinity=%.2f"), GlobalResonance, Affinity);
}

float UCosmoenergySystem::GetChannelResonance(const FString& ChannelId) const
{
    for (const FCosmoChannel& Channel : Channels)
    {
        if (Channel.Id == ChannelId)
        {
            return Channel.Resonance;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("UCosmoenergySystem: Channel '%s' not found"), *ChannelId);
    return 0.0f;
}
