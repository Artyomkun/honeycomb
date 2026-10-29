#include "ChakraSystem.h"
#include "GameManager.h"
#include "Engine/Engine.h"

UChakraSystem::UChakraSystem()
{
    ChakraColors = {
        FLinearColor::Red,
        FLinearColor(1.0f, 0.5f, 0.0f),
        FLinearColor::Yellow,
        FLinearColor::Green,
        FLinearColor::Blue,
        FLinearColor(0.294f, 0.0f, 0.51f),
        FLinearColor(1.0f, 0.0f, 1.0f),
        FLinearColor(0.0f, 1.0f, 1.0f),
        FLinearColor::White,
        FLinearColor(1.0f, 0.843f, 0.0f),
        FLinearColor::Transparent,
        FLinearColor::White
    };
}

void UChakraSystem::Initialize()
{
    if (bIsInitialized) return;
    InitializeChakras();
    bIsInitialized = true;
}

void UChakraSystem::Update(float DeltaTime)
{
    if (!bIsInitialized) return;
    RegenerateEnergy(DeltaTime);
}

void UChakraSystem::InitializeChakras()
{
    Chakras.Empty();
    GameManagerRef = AGameManager::GetInstance();

    FChakraData Chakra1;
    Chakra1.Level = 1;
    Chakra1.Name = TEXT("Муладхара (Корневая)");
    Chakra1.Description = TEXT("Основная физическая основа");
    Chakra1.ChakraColor = ChakraColors[0];
    Chakra1.Mantra = TEXT("LAM");
    Chakra1.Abilities = { TEXT("Базовая выносливость"), TEXT("Физическая сила"), TEXT("Чувство безопасности") };
    Chakra1.EnergyMultiplier = 1.0f;
    Chakra1.bIsUnlocked = true;
    Chakras.Add(Chakra1);

    FChakraData Chakra2;
    Chakra2.Level = 2;
    Chakra2.Name = TEXT("Свадхистана (Сакральная)");
    Chakra2.Description = TEXT("Центр эмоций и творчества");
    Chakra2.ChakraColor = ChakraColors[1];
    Chakra2.Mantra = TEXT("VAM");
    Chakra2.Abilities = { TEXT("Эмоциональная стабильность"), TEXT("Творческие способности"), TEXT("Сексуальная энергия") };
    Chakra2.EnergyMultiplier = 1.2f;
    Chakra2.bIsUnlocked = false;
    Chakras.Add(Chakra2);

    FChakraData Chakra3;
    Chakra3.Level = 3;
    Chakra3.Name = TEXT("Манипура (Солнечное сплетение)");
    Chakra3.Description = TEXT("Воля, сила и самодисциплина");
    Chakra3.ChakraColor = ChakraColors[2];
    Chakra3.Mantra = TEXT("RAM");
    Chakra3.Abilities = { TEXT("Сила воли"), TEXT("Лидерские качества"), TEXT("Метаболизм") };
    Chakra3.EnergyMultiplier = 1.5f;
    Chakra3.bIsUnlocked = false;
    Chakras.Add(Chakra3);

    FChakraData Chakra4;
    Chakra4.Level = 4;
    Chakra4.Name = TEXT("Анахата (Сердечная)");
    Chakra4.Description = TEXT("Любовь и сострадание");
    Chakra4.ChakraColor = ChakraColors[3];
    Chakra4.Mantra = TEXT("YAM");
    Chakra4.Abilities = { TEXT("Безусловная любовь"), TEXT("Эмпатия"), TEXT("Исцеление") };
    Chakra4.EnergyMultiplier = 2.0f;
    Chakra4.bIsUnlocked = false;
    Chakras.Add(Chakra4);

    FChakraData Chakra5;
    Chakra5.Level = 5;
    Chakra5.Name = TEXT("Вишудха (Горловая)");
    Chakra5.Description = TEXT("Коммуникация и самовыражение");
    Chakra5.ChakraColor = ChakraColors[4];
    Chakra5.Mantra = TEXT("HAM");
    Chakra5.Abilities = { TEXT("Красноречие"), TEXT("Творческое самовыражение"), TEXT("Истина") };
    Chakra5.EnergyMultiplier = 2.5f;
    Chakra5.bIsUnlocked = false;
    Chakras.Add(Chakra5);

    FChakraData Chakra6;
    Chakra6.Level = 6;
    Chakra6.Name = TEXT("Аджна (Третий глаз)");
    Chakra6.Description = TEXT("Интуиция и мудрость");
    Chakra6.ChakraColor = ChakraColors[5];
    Chakra6.Mantra = TEXT("AUM");
    Chakra6.Abilities = { TEXT("Интуиция"), TEXT("Ясновидение"), TEXT("Мудрость") };
    Chakra6.EnergyMultiplier = 3.0f;
    Chakra6.bIsUnlocked = false;
    Chakras.Add(Chakra6);

    FChakraData Chakra7;
    Chakra7.Level = 7;
    Chakra7.Name = TEXT("Сахасрара (Коронная)");
    Chakra7.Description = TEXT("Высшее сознание и духовность");
    Chakra7.ChakraColor = ChakraColors[6];
    Chakra7.Mantra = TEXT("Тишина");
    Chakra7.Abilities = { TEXT("Просветление"), TEXT("Связь с божественным"), TEXT("Единство"), TEXT("Духовность") };
    Chakra7.EnergyMultiplier = 4.0f;
    Chakra7.bIsUnlocked = false;
    Chakras.Add(Chakra7);

    FChakraData Chakra8;
    Chakra8.Level = 8;
    Chakra8.Name = TEXT("Антардхана (Скрытая)");
    Chakra8.Description = TEXT("Доступ к параллельным мирам");
    Chakra8.ChakraColor = ChakraColors[7];
    Chakra8.Mantra = TEXT("KRIM");
    Chakra8.Abilities = { TEXT("Межпространственные путешествия"), TEXT("Телепортация"), TEXT("Видение других измерений") };
    Chakra8.EnergyMultiplier = 5.0f;
    Chakra8.bIsUnlocked = false;
    Chakras.Add(Chakra8);

    FChakraData Chakra9;
    Chakra9.Level = 9;
    Chakra9.Name = TEXT("Парадхана (Высшая)");
    Chakra9.Description = TEXT("Создание реальности");
    Chakra9.ChakraColor = ChakraColors[8];
    Chakra9.Mantra = TEXT("HRIM");
    Chakra9.Abilities = { TEXT("Манипуляция реальностью"), TEXT("Создание объектов"), TEXT("Изменение времени") };
    Chakra9.EnergyMultiplier = 7.0f;
    Chakra9.bIsUnlocked = false;
    Chakras.Add(Chakra9);

    FChakraData Chakra10;
    Chakra10.Level = 10;
    Chakra10.Name = TEXT("Махадхана (Великая)");
    Chakra10.Description = TEXT("Понимание всех измерений");
    Chakra10.ChakraColor = ChakraColors[9];
    Chakra10.Mantra = TEXT("SRIM");
    Chakra10.Abilities = { TEXT("Понимание 5D"), TEXT("Доступ к Луне"), TEXT("Предварительное кураторство") };
    Chakra10.EnergyMultiplier = 10.0f;
    Chakra10.bIsUnlocked = false;
    Chakras.Add(Chakra10);

    FChakraData Chakra11;
    Chakra11.Level = 11;
    Chakra11.Name = TEXT("Парамадхана (Абсолютная)");
    Chakra11.Description = TEXT("Единство со всем существующим");
    Chakra11.ChakraColor = ChakraColors[10];
    Chakra11.Mantra = TEXT("PRAM");
    Chakra11.Abilities = { TEXT("Единство с космосом"), TEXT("Бессмертие"), TEXT("Полное понимание") };
    Chakra11.EnergyMultiplier = 15.0f;
    Chakra11.bIsUnlocked = false;
    Chakras.Add(Chakra11);

    FChakraData Chakra12;
    Chakra12.Level = 12;
    Chakra12.Name = TEXT("Атмадхана (Душевная)");
    Chakra12.Description = TEXT("Готовность стать Куратором Земель");
    Chakra12.ChakraColor = ChakraColors[11];
    Chakra12.Mantra = TEXT("ATMA");
    Chakra12.Abilities = { TEXT("Статус Куратора Земель"), TEXT("Наблюдение и баланс 54 Земель"), TEXT("Бесконечная мудрость") };
    Chakra12.EnergyMultiplier = 20.0f;
    Chakra12.bIsUnlocked = false;
    Chakras.Add(Chakra12);

    MaxEnergy = 100.0f;
    CurrentEnergy = MaxEnergy;
    ChakraMasteryMap.Empty();

    UE_LOG(LogTemp, Log, TEXT("UChakraSystem initialized: %d chakras"), Chakras.Num());
}

void UChakraSystem::OnChakraLevelUp(int32 NewLevel)
{
    if (NewLevel <= 0 || NewLevel > Chakras.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid chakra level: %d"), NewLevel);
        return;
    }

    if ((NewLevel == 4 && !HasMasteredPreviousChakras(3)) ||
        (NewLevel == 7 && !HasMasteredPreviousChakras(6)))
    {
        UE_LOG(LogTemp, Warning, TEXT("Must master previous chakras first!"));
        return;
    }

    FChakraData& Chakra = Chakras[NewLevel - 1];
    if (Chakra.bIsUnlocked)
    {
        UE_LOG(LogTemp, Log, TEXT("Chakra %s already unlocked"), *Chakra.Name);
        return;
    }

    Chakra.bIsUnlocked = true;
    MaxEnergy = 100.0f * Chakra.EnergyMultiplier;
    CurrentEnergy = MaxEnergy;

    FChakraMasteryData MasteryData;
    MasteryData.UnlockTime = FDateTime::UtcNow();
    MasteryData.MeditationCount = 0;
    MasteryData.EnergyUsed = 0.0f;
    MasteryData.TasksCompleted = 0;
    MasteryData.MasteryProgress = 0.0f;
    ChakraMasteryMap.Add(NewLevel, MasteryData);

    ActivateChakraAbilities(Chakra);

    UE_LOG(LogTemp, Log, TEXT("Chakra %s unlocked! Abilities: %s"), *Chakra.Name, *FString::Join(Chakra.Abilities, TEXT(", ")));
}

bool UChakraSystem::HasMasteredPreviousChakras(int32 UpToLevel) const
{
    for (int32 i = 1; i <= UpToLevel; i++)
    {
        if (!Chakras[i - 1].bIsUnlocked || !IsChakraMastered(i))
        {
            return false;
        }
    }
    return true;
}

bool UChakraSystem::IsChakraMastered(int32 Level) const
{
    if (Level <= 0 || Level > Chakras.Num())
    {
        return false;
    }

    const FChakraData& Chakra = Chakras[Level - 1];

    if (!Chakra.bIsUnlocked)
    {
        return false;
    }

    float EnergyThreshold = 100.0f * Chakra.EnergyMultiplier * 0.8f;
    if (CurrentEnergy < EnergyThreshold)
    {
        return false;
    }

    if (!ChakraMasteryMap.Contains(Level))
    {
        return false;
    }

    const FChakraMasteryData& MasteryData = ChakraMasteryMap[Level];

    if (MasteryData.MeditationCount < 3)
    {
        return false;
    }

    if (MasteryData.EnergyUsed < 50.0f * Chakra.EnergyMultiplier)
    {
        return false;
    }

    FDateTime Now = FDateTime::UtcNow();
    FTimespan TimeSinceUnlock = Now - MasteryData.UnlockTime;
    if (TimeSinceUnlock.GetTotalHours() < 2.0f)
    {
        return false;
    }

    return true;
}

void UChakraSystem::ActivateChakraAbilities(const FChakraData& Chakra)
{
    switch (Chakra.Level)
    {
    case 4:
        break;
    case 6:
        break;
    case 8:
        break;
    case 10:
        if (GameManagerRef)
        {
            GameManagerRef->CheckMoonAccess();
        }
        break;
    default:
        break;
    }
}

bool UChakraSystem::IsChakraUnlocked(int32 Level) const
{
    if (Level > 0 && Level <= Chakras.Num())
    {
        return Chakras[Level - 1].bIsUnlocked;
    }
    return false;
}

FChakraData UChakraSystem::GetChakraData(int32 Level) const
{
    if (Level > 0 && Level <= Chakras.Num())
    {
        return Chakras[Level - 1];
    }
    return FChakraData();
}

void UChakraSystem::UseEnergy(float Amount)
{
    CurrentEnergy = FMath::Max(0.0f, CurrentEnergy - Amount);
}

void UChakraSystem::RegenerateEnergy(float DeltaTime)
{
    if (CurrentEnergy < MaxEnergy)
    {
        CurrentEnergy += EnergyRegenerationRate * DeltaTime;
        CurrentEnergy = FMath::Min(CurrentEnergy, MaxEnergy);
    }
}

float UChakraSystem::GetEnergyPercentage() const
{
    return MaxEnergy > 0.0f ? CurrentEnergy / MaxEnergy : 0.0f;
}

bool UChakraSystem::CanUseAbility(float EnergyCost) const
{
    return CurrentEnergy >= EnergyCost;
}

void UChakraSystem::AddEnergy(float Amount)
{
    CurrentEnergy = FMath::Min(CurrentEnergy + Amount, MaxEnergy);
    UE_LOG(LogTemp, Verbose, TEXT("Energy added: %.1f, Current: %.1f / %.1f"), Amount, CurrentEnergy, MaxEnergy);
}

void UChakraSystem::SetMaxEnergy(float NewMax)
{
    MaxEnergy = NewMax;
    CurrentEnergy = FMath::Min(CurrentEnergy, MaxEnergy);
    UE_LOG(LogTemp, Verbose, TEXT("Max energy set to: %.1f"), MaxEnergy);
}

int32 UChakraSystem::GetCurrentChakraLevel() const
{
    for (int32 i = Chakras.Num() - 1; i >= 0; i--)
    {
        if (Chakras[i].bIsUnlocked)
        {
            return Chakras[i].Level;
        }
    }
    return 1;
}

TArray<int32> UChakraSystem::GetUnlockedChakras() const
{
    TArray<int32> Result;
    for (const FChakraData& Chakra : Chakras)
    {
        if (Chakra.bIsUnlocked)
        {
            Result.Add(Chakra.Level);
        }
    }
    return Result;
}

float UChakraSystem::GetEnergyRegenerationRate() const
{
    return EnergyRegenerationRate;
}

void UChakraSystem::SetEnergyRegenerationRate(float Rate)
{
    EnergyRegenerationRate = FMath::Max(0.0f, Rate);
    UE_LOG(LogTemp, Verbose, TEXT("Energy regeneration rate set to: %.1f"), EnergyRegenerationRate);
}
