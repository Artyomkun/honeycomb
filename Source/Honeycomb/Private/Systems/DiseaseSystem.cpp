#include "DiseaseSystem.h"
#include "MoodSystem.h"
#include "ChakraSystem.h"
#include "KarmaSystem.h"
#include "DiseaseCatalog.h"
#include "GameManager.h"
#include "Engine/Engine.h"

UDiseaseSystem::UDiseaseSystem()
{
}

void UDiseaseSystem::Initialize()
{
    if (bIsInitialized) return;

    if (DiseaseCatalogRef)
    {
        DiseaseCatalogRef->Load();
    }

    bIsInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("UDiseaseSystem initialized"));
}

void UDiseaseSystem::Update(float DeltaTime)
{
    if (!bIsInitialized) return;

    for (int32 i = ActiveDiseases.Num() - 1; i >= 0; i--)
    {
        FDisease& D = ActiveDiseases[i];
        float Delta = GetProgressDelta(D);
        D.Severity = FMath::Clamp(D.Severity + Delta * DeltaTime, 0.0f, 1.0f);

        if (D.Severity <= 0.01f)
        {
            D.bIsActive = false;
            ActiveDiseases.RemoveAt(i);
        }
    }
}

void UDiseaseSystem::AddDisease(const FDisease& NewDisease)
{
    FDisease D = NewDisease;
    D.bIsActive = true;
    D.Severity = FMath::Max(D.Severity, 0.1f);
    ActiveDiseases.Add(D);

    UE_LOG(LogTemp, Warning, TEXT("FDisease added: %s (Severity: %.2f)"), *D.Name, D.Severity);
}

void UDiseaseSystem::CureDisease(const FString& Id, float Amount)
{
    for (int32 i = 0; i < ActiveDiseases.Num(); i++)
    {
        if (ActiveDiseases[i].Id == Id)
        {
            FDisease& D = ActiveDiseases[i];
            D.Severity = FMath::Clamp(D.Severity - Amount, 0.0f, 1.0f);

            if (D.Severity <= 0.01f)
            {
                D.bIsActive = false;
                ActiveDiseases.RemoveAt(i);
                UE_LOG(LogTemp, Log, TEXT("FDisease cured: %s"), *D.Name);
            }
            else
            {
                UE_LOG(LogTemp, Log, TEXT("FDisease %s severity reduced to %.2f"), *D.Name, D.Severity);
            }
            return;
        }
    }
}

float UDiseaseSystem::GetTotalSeverity() const
{
    float Total = 0.0f;
    for (const FDisease& D : ActiveDiseases)
    {
        Total += D.Severity;
    }
    return Total;
}

float UDiseaseSystem::GetBodyTemperature() const
{
    float BaseTemp = 36.6f;

    for (const FDisease& D : ActiveDiseases)
    {
        if (D.Type == DiseaseType::Physical)
        {
            BaseTemp += D.Severity * 1.5f;
        }
    }

    return FMath::Clamp(BaseTemp, 35.0f, 41.0f);
}

bool UDiseaseSystem::HasActiveDiseases() const
{
    return ActiveDiseases.Num() > 0;
}

TArray<FDisease> UDiseaseSystem::GetActiveDiseases() const
{
    return ActiveDiseases;
}

float UDiseaseSystem::GetProgressDelta(const FDisease& D)
{
    float BaseDelta = 0.0f;

    if (!MoodSystemRef) return BaseDelta;

    float Fear = MoodSystemRef->GetFear();
    float Anger = MoodSystemRef->GetAnger();
    float Sadness = MoodSystemRef->GetSadness();
    float Stress = MoodSystemRef->GetStress();
    float Calm = MoodSystemRef->GetCalm();
    float Love = MoodSystemRef->GetLove();
    float Joy = MoodSystemRef->GetJoy();
    float Empathy = MoodSystemRef->GetEmpathy();
    float Spirituality = MoodSystemRef->GetSpirituality();

    float NegativeLoad = (Fear + Anger + Sadness + Stress) / 4.0f;
    float PositiveLoad = (Calm + Love + Joy) / 3.0f;

    switch (D.Type)
    {
        case DiseaseType::Physical:
            BaseDelta += 0.15f * (NegativeLoad - PositiveLoad);
            BaseDelta -= 0.05f * GetDiscipline();
            break;

        case DiseaseType::Psychological:
            BaseDelta += 0.25f * (NegativeLoad - PositiveLoad);
            BaseDelta -= 0.1f * Empathy;
            BaseDelta -= 0.1f * Love;
            break;

        case DiseaseType::Mental:
            BaseDelta += 0.2f * (Stress - Calm);
            BaseDelta -= 0.07f * GetDiscipline();
            break;

        case DiseaseType::Spiritual:
            BaseDelta += 0.2f * (0.5f - Spirituality);
            BaseDelta -= 0.12f * Love;
            BaseDelta -= 0.12f * Spirituality;
            break;

        case DiseaseType::Astral:
            BaseDelta += 0.18f * (NegativeLoad - PositiveLoad);
            BaseDelta -= 0.05f * GetChakraBalanceFactor();
            break;

        default:
            break;
    }

    if (KarmaSystemRef)
    {
        KarmaTier Tier = KarmaSystemRef->GetKarmaTier();
        switch (Tier)
        {
            case KarmaTier::Radiant:
                BaseDelta -= 0.08f;
                break;
            case KarmaTier::Light:
                BaseDelta -= 0.04f;
                break;
            case KarmaTier::Shadow:
                BaseDelta += 0.04f;
                break;
            case KarmaTier::Dark:
                BaseDelta += 0.08f;
                break;
            default:
                break;
        }
    }

    return BaseDelta;
}

float UDiseaseSystem::GetDiscipline() const
{
    if (MoodSystemRef)
    {
        return MoodSystemRef->GetDiscipline();
    }
    return 0.5f;
}

float UDiseaseSystem::GetChakraBalanceFactor() const
{
    if (!ChakraSystemRef) return 0.5f;

    float EnergyPercent = ChakraSystemRef->GetEnergyPercentage();
    int32 Level = ChakraSystemRef->GetCurrentChakraLevel();

    return FMath::Clamp((Level / 12.0f) * 0.5f + EnergyPercent * 0.5f, 0.0f, 1.0f);
}
