#include "ExtendedChakraSystem.h"
#include "BirthProfileSystem.h"
#include "CosmoenergySystem.h"
#include "Engine/Engine.h"

UExtendedChakraSystem::UExtendedChakraSystem()
{
    constexpr int32 NumChakras = 12;
    ChakraLevels.Init(0, NumChakras);
    ChakraEnergies.Init(0.0f, NumChakras);

    CurrentChakraIndex = 0;
    TotalEnergy = 0.0f;
    bKundaliniAwakened = false;
    BaseChakraSystem = nullptr;

    UE_LOG(LogTemp, Verbose, TEXT("UExtendedChakraSystem: Constructor initialized"));
}

void UExtendedChakraSystem::Initialize154()
{
    if (bIsInitialized) return;

    Chakras.Empty();
    Chakras.Reserve(154);

    for (int32 i = 0; i < 154; i++)
    {
        FExtendedChakra Ch;
        Ch.Index = i;
        Ch.Name = FString::Printf(TEXT("Чакра %d"), i + 1);
        Ch.Mantra = TEXT("OM");
        Ch.EnergyMultiplier = 1.0f + i * 0.02f;
        Ch.bUnlocked = false;

        switch (i)
        {
            case 3:
                Ch.Abilities.Add(TEXT("Любовь"));
                break;
            case 5:
                Ch.Mantra = TEXT("Тишина");
                break;
            case 6:
                Ch.Mantra = TEXT("AUM");
                Ch.Abilities.Add(TEXT("Духовность"));
                break;
            case 10:
                Ch.Mantra = TEXT("KRIM");
                Ch.Abilities.Add(TEXT("Межпространственные путешествия"));
                break;
            case 14:
                Ch.Mantra = TEXT("HRIM");
                Ch.Abilities.Add(TEXT("Создание реальности"));
                break;
            case 20:
                Ch.Mantra = TEXT("SRIM");
                Ch.Abilities.Add(TEXT("Понимание 5D"));
                break;
            case 30:
                Ch.Mantra = TEXT("PRAM");
                Ch.Abilities.Add(TEXT("Единство с космосом"));
                break;
            case 50:
                Ch.Mantra = TEXT("ATMA");
                Ch.Abilities.Add(TEXT("Статус Куратора"));
                break;
            default:
                break;
        }

        Chakras.Add(Ch);
    }

    int32 Bias = 0;
    if (FBirthProfile)
    {
        Bias = FBirthProfile->GetInitialChakraBiasIndex();
    }

    CurrentChakraIndex = FMath::Clamp(Bias, 0, 153);
    Chakras[CurrentChakraIndex].bUnlocked = true;

    bIsInitialized = true;

    UE_LOG(LogTemp, Log, TEXT("UExtendedChakraSystem: 154 chakras initialized, starting at index %d"), CurrentChakraIndex);
}

FExtendedChakra UExtendedChakraSystem::GetChakraByIndex(int32 Index) const
{
    if (Index >= 0 && Index < Chakras.Num())
    {
        return Chakras[Index];
    }
    return FExtendedChakra();
}

void UExtendedChakraSystem::SetCurrentChakraIndex(int32 NewIndex)
{
    if (NewIndex < 0 || NewIndex >= Chakras.Num()) return;

    CurrentChakraIndex = NewIndex;
    Chakras[CurrentChakraIndex].bUnlocked = true;

    UE_LOG(LogTemp, Log, TEXT("Current chakra set to index: %d"), CurrentChakraIndex);
}

void UExtendedChakraSystem::UnlockChakra(int32 Index)
{
    if (Index < 0 || Index >= Chakras.Num()) return;

    if (!Chakras[Index].bUnlocked)
    {
        Chakras[Index].bUnlocked = true;
        UE_LOG(LogTemp, Log, TEXT("Chakra %d unlocked: %s"), Index, *Chakras[Index].Name);
    }
}

bool UExtendedChakraSystem::IsChakraUnlocked(int32 Index) const
{
    if (Index < 0 || Index >= Chakras.Num()) return false;
    return Chakras[Index].bUnlocked;
}

int32 UExtendedChakraSystem::GetUnlockedCount() const
{
    int32 Count = 0;
    for (const FExtendedChakra& Ch : Chakras)
    {
        if (Ch.bUnlocked) Count++;
    }
    return Count;
}
