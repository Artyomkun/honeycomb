#include "KriyaYogaSystem.h"
#include "ChakraSystem.h"
#include "GameManager.h"
#include "Engine/Engine.h"

UKriyaYogaSystem::UKriyaYogaSystem()
{
    for (int32 i = 0; i < 12; i++)
    {
        ChakraActivated[i] = false;
        ChakraEnergyLevels[i] = 0.0f;
        KundaliniFlow[i] = 0.0f;
    }
}

void UKriyaYogaSystem::Initialize()
{
    if (bIsInitialized) return;

    for (int32 i = 0; i < 12; i++)
    {
        ChakraActivated[i] = false;
        ChakraEnergyLevels[i] = 0.0f;
        KundaliniFlow[i] = 0.0f;
    }

    InitializeKriyaTechniques();
    bKundaliniAwakened = false;
    bIsInitialized = true;

    UE_LOG(LogTemp, Log, TEXT("UKriyaYogaSystem initialized"));
}

void UKriyaYogaSystem::InitializeKriyaTechniques()
{
    KriyaTechniques.Empty();

    KriyaTechniques.Add(1, FKriyaTechnique(
        TEXT("Муладхара Крийя"), TEXT("LAM"),
        TEXT("Пробуждение корневой чакры"), 1.0f,
        TEXT("Заземление и стабильность")
    ));

    KriyaTechniques.Add(2, FKriyaTechnique(
        TEXT("Свадхистана Крийя"), TEXT("VAM"),
        TEXT("Активация сакральной энергии"), 1.2f,
        TEXT("Творчество и эмоции")
    ));

    KriyaTechniques.Add(3, FKriyaTechnique(
        TEXT("Манипура Крийя"), TEXT("RAM"),
        TEXT("Пробуждение солнечного сплетения"), 1.5f,
        TEXT("Воля и сила")
    ));

    KriyaTechniques.Add(4, FKriyaTechnique(
        TEXT("Анахата Крийя"), TEXT("YAM"),
        TEXT("Открытие сердечной чакры"), 2.0f,
        TEXT("Любовь и сострадание")
    ));

    KriyaTechniques.Add(5, FKriyaTechnique(
        TEXT("Вишудха Крийя"), TEXT("HAM"),
        TEXT("Активация горловой чакры"), 2.5f,
        TEXT("Коммуникация и истина")
    ));

    KriyaTechniques.Add(6, FKriyaTechnique(
        TEXT("Аджна Крийя"), TEXT("Тишина"),
        TEXT("Пробуждение третьего глаза"), 3.0f,
        TEXT("Интуиция и мудрость")
    ));

    KriyaTechniques.Add(7, FKriyaTechnique(
        TEXT("Сахасрара Крийя"), TEXT("Тишина"),
        TEXT("Открытие коронной чакры"), 4.0f,
        TEXT("Духовность и единство")
    ));

    KriyaTechniques.Add(8, FKriyaTechnique(
        TEXT("Антардхана Крийя"), TEXT("KRIM"),
        TEXT("Межпространственные путешествия"), 5.0f,
        TEXT("Доступ к параллельным мирам")
    ));

    KriyaTechniques.Add(9, FKriyaTechnique(
        TEXT("Парадхана Крийя"), TEXT("HRIM"),
        TEXT("Создание реальности"), 7.0f,
        TEXT("Манипуляция материей")
    ));

    KriyaTechniques.Add(10, FKriyaTechnique(
        TEXT("Махадхана Крийя"), TEXT("SRIM"),
        TEXT("Понимание 5D"), 10.0f,
        TEXT("Доступ к Луне")
    ));

    KriyaTechniques.Add(11, FKriyaTechnique(
        TEXT("Парамадхана Крийя"), TEXT("PRAM"),
        TEXT("Единство с космосом"), 15.0f,
        TEXT("Бессмертие")
    ));

    KriyaTechniques.Add(12, FKriyaTechnique(
        TEXT("Атмадхана Крийя"), TEXT("ATMA"),
        TEXT("Статус Куратора"), 20.0f,
        TEXT("Бесконечная мудрость")
    ));
}

void UKriyaYogaSystem::Update(float DeltaTime)
{
    if (!bIsInitialized) return;

    if (bIsKriyaActive)
    {
        UpdateKriyaPractice(DeltaTime);
    }

    UpdatePranaFlow(DeltaTime);
    UpdateKundaliniFlow(DeltaTime);
}

void UKriyaYogaSystem::StartKriyaPractice(int32 KriyaLevel)
{
    if (KriyaLevel < 1 || KriyaLevel > 12) return;

    if (!CanPracticeKriya(KriyaLevel))
    {
        UE_LOG(LogTemp, Warning, TEXT("Kriya level %d not available"), KriyaLevel);
        return;
    }

    CurrentKriyaLevel = KriyaLevel;
    bIsKriyaActive = true;
    CurrentKriyaTime = 0.0f;

    ActivateChakra(KriyaLevel);
    ApplyKriyaEffects(true);

    const FKriyaTechnique* Tech = KriyaTechniques.Find(KriyaLevel);
    if (Tech)
    {
        UE_LOG(LogTemp, Log, TEXT("Starting Kriya: %s | Mantra: %s | Effect: %s"), *Tech->Name, *Tech->Mantra, *Tech->Effect);
    }
}

void UKriyaYogaSystem::StopKriyaPractice()
{
    if (!bIsKriyaActive) return;

    bIsKriyaActive = false;
    ApplyKriyaEffects(false);
    CheckKriyaProgress();

    UE_LOG(LogTemp, Log, TEXT("Kriya practice stopped"));
}

void UKriyaYogaSystem::UpdateKriyaPractice(float DeltaTime)
{
    if (!bIsKriyaActive) return;

    float TimeMultiplier = CalculateKriyaMultiplier();
    CurrentKriyaTime += DeltaTime * TimeMultiplier;

    const FKriyaTechnique* Tech = KriyaTechniques.Find(CurrentKriyaLevel);
    if (!Tech) return;

    float KundaliniGain = DeltaTime * Tech->EnergyMultiplier * KriyaEnergyMultiplier;
    KundaliniEnergy = FMath::Min(KundaliniAwakeningThreshold, KundaliniEnergy + KundaliniGain);

    int32 Index = CurrentKriyaLevel - 1;
    ChakraEnergyLevels[Index] += DeltaTime * Tech->EnergyMultiplier;

    if (KundaliniEnergy >= KundaliniAwakeningThreshold)
    {
        AwakenKundalini();
    }

    float RequiredTime = GetRequiredKriyaTime(CurrentKriyaLevel);
    if (CurrentKriyaTime >= RequiredTime)
    {
        CompleteKriyaPractice();
    }
}

float UKriyaYogaSystem::CalculateKriyaMultiplier() const
{
    float Multiplier = 1.0f;

    if (GameManagerRef && CurrentKriyaLevel == GameManagerRef->GetCurrentChakraLevel())
    {
        Multiplier *= 1.5f;
    }

    int32 ActivatedCount = 0;
    for (int32 i = 0; i < CurrentKriyaLevel; i++)
    {
        if (ChakraActivated[i]) ActivatedCount++;
    }

    Multiplier *= (1.0f + ActivatedCount * 0.1f);
    return Multiplier;
}

bool UKriyaYogaSystem::CanPracticeKriya(int32 KriyaLevel) const
{
    if (KriyaLevel <= 3) return true;

    if (KriyaLevel == 4)
    {
        return HasMasteredChakras(1, 3);
    }

    if (KriyaLevel == 7)
    {
        return HasMasteredChakras(1, 6);
    }

    if (GameManagerRef)
    {
        // Allow attempting the next chakra as well, so progression cannot
        // dead-end: activating a chakra is what advances GameManager.
        return GameManagerRef->GetCurrentChakraLevel() + 1 >= KriyaLevel;
    }

    return false;
}

bool UKriyaYogaSystem::HasMasteredChakras(int32 FromLevel, int32 ToLevel) const
{
    for (int32 i = FromLevel; i <= ToLevel; i++)
    {
        if (!ChakraActivated[i - 1]) return false;
    }
    return true;
}

void UKriyaYogaSystem::ActivateChakra(int32 ChakraLevel)
{
    if (ChakraLevel > 0 && ChakraLevel <= 12)
    {
        ChakraActivated[ChakraLevel - 1] = true;
        UE_LOG(LogTemp, Log, TEXT("Chakra %d activated via Kriya"), ChakraLevel);
    }
}

void UKriyaYogaSystem::ApplyKriyaEffects(bool bIsStarting)
{
    if (bIsStarting)
    {
        UE_LOG(LogTemp, Log, TEXT("Kriya effects applied"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Kriya effects removed"));
    }
}

void UKriyaYogaSystem::UpdatePranaFlow(float DeltaTime)
{
    for (int32 i = 0; i < 12; i++)
    {
        if (ChakraActivated[i])
        {
            PranaEnergy += DeltaTime * PranaFlowRate * (i + 1) * 0.1f;
        }
    }
}

void UKriyaYogaSystem::UpdateKundaliniFlow(float DeltaTime)
{
    for (int32 i = 0; i < 12; i++)
    {
        if (ChakraActivated[i])
        {
            KundaliniFlow[i] += DeltaTime * KundaliniEnergy * 0.01f;
        }
    }
}

void UKriyaYogaSystem::AwakenKundalini()
{
    // Kundalini awakens exactly once, otherwise the energy cap would re-fire it
    // every frame and skip the whole chakra ladder.
    if (bKundaliniAwakened)
    {
        return;
    }
    bKundaliniAwakened = true;

    UE_LOG(LogTemp, Log, TEXT("KriyaYogaSystem: Kundalini awakened"));

    static constexpr int32 NumChakras = 12;
    for (int32 i = 0; i < NumChakras; i++)
    {
        ChakraActivated[i] = true;
        ChakraEnergyLevels[i] = MaxChakraEnergy;
    }

    if (ChakraSystemRef)
    {
        ChakraSystemRef->ApplyKundaliniBonus();
    }

    if (GameManagerRef)
    {
        GameManagerRef->OnKundaliniAwakened();
    }
}

float UKriyaYogaSystem::GetRequiredKriyaTime(int32 KriyaLevel) const
{
    static const float Times[] = { 5.0f, 10.0f, 15.0f, 20.0f, 25.0f, 30.0f, 40.0f, 50.0f, 60.0f, 75.0f, 90.0f, 120.0f };
    if (KriyaLevel > 0 && KriyaLevel <= 12) return Times[KriyaLevel - 1];
    return 30.0f;
}

void UKriyaYogaSystem::CheckKriyaProgress()
{
    float Progress = GetKriyaProgress();

    if (Progress >= 0.9f)
    {
        UE_LOG(LogTemp, Log, TEXT("Excellent Kriya! Progress: %.0f%%"), Progress * 100.0f);
    }
    else if (Progress >= 0.6f)
    {
        UE_LOG(LogTemp, Log, TEXT("Good Kriya. Progress: %.0f%%"), Progress * 100.0f);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Short Kriya. Progress: %.0f%%"), Progress * 100.0f);
    }
}

void UKriyaYogaSystem::CompleteKriyaPractice()
{
    if (CurrentKriyaLevel <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("KriyaYogaSystem: Invalid Kriya level: %d"), CurrentKriyaLevel);
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("KriyaYogaSystem: Kriya level %d complete"), CurrentKriyaLevel);

    const FKriyaTechnique* Tech = KriyaTechniques.Find(CurrentKriyaLevel);
    if (!Tech)
    {
        UE_LOG(LogTemp, Warning, TEXT("KriyaYogaSystem: Technique not found for level %d"), CurrentKriyaLevel);
        return;
    }

    if (ChakraSystemRef)
    {
        float EnergyBonus = Tech->EnergyMultiplier * MaxChakraEnergy;
        ChakraSystemRef->AddEnergy(EnergyBonus);
        UE_LOG(LogTemp, Log, TEXT("KriyaYogaSystem: Applied energy bonus: %f"), EnergyBonus);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("KriyaYogaSystem: ChakraSystemRef not found"));
    }

    CheckChakraAdvancement();
    StopKriyaPractice();
    OnKriyaComplete.Broadcast(CurrentKriyaLevel);
}

void UKriyaYogaSystem::CheckChakraAdvancement()
{
    if (!GameManagerRef) return;

    int32 ActivatedCount = 0;
    for (int32 i = 0; i < 12; i++)
    {
        if (ChakraActivated[i]) ActivatedCount++;
    }

    if (ActivatedCount >= GameManagerRef->GetCurrentChakraLevel())
    {
        GameManagerRef->AdvanceChakraLevel();
    }
}

float UKriyaYogaSystem::GetKriyaProgress() const
{
    float Required = GetRequiredKriyaTime(CurrentKriyaLevel);
    return Required > 0.0f ? CurrentKriyaTime / Required : 0.0f;
}

bool UKriyaYogaSystem::IsChakraActivated(int32 ChakraLevel) const
{
    if (ChakraLevel > 0 && ChakraLevel <= 12)
    {
        return static_cast<bool>(ChakraActivated[ChakraLevel - 1]);
    }
    return false;
}

FString UKriyaYogaSystem::GetCurrentTechniqueName() const
{
    const FKriyaTechnique* Tech = KriyaTechniques.Find(CurrentKriyaLevel);
    return Tech ? Tech->Name : TEXT("Неизвестно");
}

bool UKriyaYogaSystem::IsKriyaUnlocked(int32 KriyaLevel) const
{
    return CanPracticeKriya(KriyaLevel);
}

float UKriyaYogaSystem::GetChakraEnergyLevel(int32 Index) const
{
    if (Index >= 0 && Index < 12) return ChakraEnergyLevels[Index];
    return 0.0f;
}
