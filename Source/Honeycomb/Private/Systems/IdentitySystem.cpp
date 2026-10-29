#include "IdentitySystem.h"
#include "Engine/Engine.h"

UIdentitySystem::UIdentitySystem()
{
    CurrentIdentity = EIdentityType::None;
    IdentityLevel = 0;
    IdentityProgress = 0.0f;
    IdentityHistory.Empty();
    MaxHistorySize = 50;
    GameManager = nullptr;
    PlayerController = nullptr;
    ChakraSystem = nullptr;

    UE_LOG(LogTemp, Verbose, TEXT("UIdentitySystem: Constructor initialized"));
}

void UIdentitySystem::SetPlayerName(const FString& NewName)
{
    if (NewName.IsEmpty()) return;
    PlayerName = NewName;
    UE_LOG(LogTemp, Log, TEXT("Player name set to: %s"), *PlayerName);
}

void UIdentitySystem::RequestChangeGender(GenderIdentity NewGender, bool bConfirm)
{
    if (!bConfirm)
    {
        UE_LOG(LogTemp, Log, TEXT("Gender change cancelled"));
        return;
    }

    PendingGender = NewGender;
    FString GenderName = UEnum::GetValueAsString(NewGender);
    ShowConfirmationDialog(FString::Printf(TEXT("Вы действительно хотите изменить пол на %s?"), *GenderName), EIdentityChangeType::Gender, bConfirm);
}

void UIdentitySystem::RequestChangeReligion(Religion NewReligion, bool bConfirm)
{
    if (!bConfirm)
    {
        UE_LOG(LogTemp, Log, TEXT("Religion change cancelled"));
        return;
    }

    PendingReligion = NewReligion;
    FString ReligionName = UEnum::GetValueAsString(NewReligion);
    ShowConfirmationDialog(FString::Printf(TEXT("Вы действительно хотите изменить религию на %s?"), *ReligionName), EIdentityChangeType::Religion, bConfirm);
}

void UIdentitySystem::RequestChangeRelationship(RelationshipType NewType, bool bConfirm)
{
    if (!bConfirm)
    {
        UE_LOG(LogTemp, Log, TEXT("Relationship change cancelled"));
        return;
    }

    PendingRelationship = NewType;
    FString TypeName = UEnum::GetValueAsString(NewType);
    ShowConfirmationDialog(FString::Printf(TEXT("Вы действительно хотите изменить тип отношений на %s?"), *TypeName), EIdentityChangeType::Relationship, bConfirm);
}

void UIdentitySystem::ShowConfirmationDialog(
    const FString& Message,
    EIdentityChangeType ChangeType,
    bool bConfirm)
{
    if (!bConfirm)
    {
        PendingGender.Reset();
        OnChangeCancelled.Broadcast(ChangeType);
        return;
    }

    switch (ChangeType)
    {
        case EIdentityChangeType::Gender:
            ApplyGenderChange();
            break;

        case EIdentityChangeType::Religion:
            ApplyReligionChange();
            break;

        case EIdentityChangeType::Relationship:
            ApplyRelationshipChange();
            break;

        case EIdentityChangeType::None:
        default:
            UE_LOG(LogTemp, Warning, TEXT("UIdentitySystem: Unknown change type %d"), (int32)ChangeType);
            break;
    }
}

void UIdentitySystem::ApplyGenderChange()
{
    if (!PendingGender.IsSet())
    {
        UE_LOG(LogTemp, Warning, TEXT("UIdentitySystem: No pending gender change"));
        return;
    }

    Gender = PendingGender.GetValue();
    OnGenderChanged.Broadcast(Gender);
    PendingGender.Reset();

    UE_LOG(LogTemp, Log, TEXT("UIdentitySystem: Gender changed to %d"), (int32)Gender);
}

void UIdentitySystem::ApplyReligionChange()
{
    if (!PendingReligion.IsSet())
    {
        UE_LOG(LogTemp, Warning, TEXT("UIdentitySystem: No pending religion change"));
        return;
    }

    ReligionValue = PendingReligion.GetValue();
    OnReligionChanged.Broadcast(ReligionValue);
    PendingReligion.Reset();

    UE_LOG(LogTemp, Log, TEXT("UIdentitySystem: Religion changed to %d"), (int32)ReligionValue);
}

void UIdentitySystem::ApplyRelationshipChange()
{
    if (!PendingRelationship.IsSet())
    {
        UE_LOG(LogTemp, Warning, TEXT("UIdentitySystem: No pending relationship change"));
        return;
    }

    Relationship = PendingRelationship.GetValue();
    OnRelationshipChanged.Broadcast(Relationship);
    PendingRelationship.Reset();

    UE_LOG(LogTemp, Log, TEXT("UIdentitySystem: Relationship changed to %d"), (int32)Relationship);
}

void UIdentitySystem::SetOpenness(float Value)
{
    PersonalityData.Openness = FMath::Clamp(Value, 0.0f, 1.0f);
}

void UIdentitySystem::SetConscientiousness(float Value)
{
    PersonalityData.Conscientiousness = FMath::Clamp(Value, 0.0f, 1.0f);
}

void UIdentitySystem::SetExtraversion(float Value)
{
    PersonalityData.Extraversion = FMath::Clamp(Value, 0.0f, 1.0f);
}

void UIdentitySystem::SetAgreeableness(float Value)
{
    PersonalityData.Agreeableness = FMath::Clamp(Value, 0.0f, 1.0f);
}

void UIdentitySystem::SetNeuroticism(float Value)
{
    PersonalityData.Neuroticism = FMath::Clamp(Value, 0.0f, 1.0f);
}
