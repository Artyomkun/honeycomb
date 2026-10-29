#include "BirthProfileSystem.h"
#include "Engine/Engine.h"
#include "Misc/DateTime.h"
#include "Math/RandomStream.h"

UBirthProfileSystem::UBirthProfileSystem()
{
    BirthDate = FDateTime::Now();
    BirthPlace.Empty();
    BirthTime.Empty();
    BirthDayOfWeek = 0;
    AstrologyChart.Empty();
    bIsInitialized = false;

    UE_LOG(LogTemp, Verbose, TEXT("UBirthProfileSystem: Constructor initialized"));
}

void UBirthProfileSystem::Initialize(FDateTime BirthDateUtc, const FString& PlaceName)
{
    Profile.BirthDateUtc = BirthDateUtc;
    Profile.BirthPlace = PlaceName;
    Profile.SoulMatrixSeed = ComputeSoulMatrixSeed(BirthDateUtc, PlaceName);
    Profile.LifePath = ComputeLifePathNumber(BirthDateUtc);

    DeriveModifiers();

    UE_LOG(LogTemp, Log, TEXT("FBirthProfile initialized: SoulMatrixSeed=%d, LifePath=%d"),
        Profile.SoulMatrixSeed, Profile.LifePath);
}

int32 UBirthProfileSystem::ComputeSoulMatrixSeed(FDateTime Date, const FString& Place)
{
    int32 Hash = 17;
    Hash = Hash * 31 + Date.GetYear();
    Hash = Hash * 31 + Date.GetMonth();
    Hash = Hash * 31 + Date.GetDay();
    Hash = Hash * 31 + Date.GetHour();
    Hash = Hash * 31 + Date.GetMinute();
    Hash = Hash * 31 + (Place.IsEmpty() ? 0 : GetTypeHash(Place));

    return FMath::Abs(Hash);
}

int32 UBirthProfileSystem::ComputeLifePathNumber(FDateTime Date)
{
    FString DateStr = FString::Printf(TEXT("%04d%02d%02d"),
        Date.GetYear(), Date.GetMonth(), Date.GetDay());

    int32 Sum = 0;
    for (int32 i = 0; i < DateStr.Len(); i++)
    {
        TCHAR Char = DateStr[i];
        if (FChar::IsDigit(Char))
        {
            Sum += Char - '0';
        }
    }

    while (Sum > 9)
    {
        int32 NewSum = 0;
        while (Sum > 0)
        {
            NewSum += Sum % 10;
            Sum /= 10;
        }
        Sum = NewSum;
    }

    return FMath::Max(1, Sum);
}

void UBirthProfileSystem::DeriveModifiers()
{
    FRandomStream Random;
    Random.Initialize(Profile.SoulMatrixSeed);

    InitialEnergyBonus = Random.FRandRange(0.05f, 0.25f) + Profile.LifePath * 0.01f;
    InitialChakraBiasIndex = Random.RandRange(0, 153);
    CosmoChannelAffinity = FMath::Clamp(0.3f + Profile.LifePath * 0.05f, 0.0f, 1.0f);

    UE_LOG(LogTemp, Log, TEXT("Derived modifiers: EnergyBonus=%.2f, ChakraBias=%d, CosmoAffinity=%.2f"), InitialEnergyBonus, InitialChakraBiasIndex, CosmoChannelAffinity);
}
