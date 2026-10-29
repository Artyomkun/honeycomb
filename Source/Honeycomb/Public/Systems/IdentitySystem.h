#pragma once

#include "HoneycombCore.h"
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "IdentitySystem.generated.h"

class AGameManager;
class AVRPlayerController;
class UChakraSystem;

UENUM(BlueprintType)
enum class GenderIdentity: uint8
{
    Male,
    Female,
    NonBinary,
    Agender,
    Bigender,
    Genderfluid,
    Other
};

UENUM(BlueprintType)
enum class RelationshipType: uint8
{
    Single,
    Monogamous,
    Polyamorous,
    Aromantic,
    Asexual,
    OpenRelationship,
    CivilUnion,
    Marriage
};

UENUM(BlueprintType)
enum class Religion: uint8
{
    Atheism,
    Agnosticism,
    Christianity,
    Islam,
    Judaism,
    Buddhism,
    Hinduism,
    Sikhism,
    Taoism,
    Shinto,
    Paganism,
    Other
};

UENUM(BlueprintType)
enum class EIdentityType: uint8
{
    None,
    Physical,
    Emotional,
    Mental,
    Spiritual,
    Cosmic
};

UENUM(BlueprintType)
enum class EIdentityChangeType: uint8
{
    None,
    Gender,
    Religion,
    Relationship
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChangeCancelled, EIdentityChangeType, ChangeType);

USTRUCT(BlueprintType)
struct FPersonality
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    float Openness = 0.5f;

    UPROPERTY(BlueprintReadWrite)
    float Conscientiousness = 0.5f;

    UPROPERTY(BlueprintReadWrite)
    float Extraversion = 0.5f;

    UPROPERTY(BlueprintReadWrite)
    float Agreeableness = 0.5f;

    UPROPERTY(BlueprintReadWrite)
    float Neuroticism = 0.5f;

    UPROPERTY(BlueprintReadWrite)
    TArray<FString> Biases;
};

UCLASS(BlueprintType, Blueprintable)
class HONEYCOMB_API UIdentitySystem: public UObject
{
    GENERATED_BODY()

    public:
        UIdentitySystem();

        UFUNCTION(BlueprintCallable, Category = "Identity")
        void SetPlayerName(const FString& NewName);

        UFUNCTION(BlueprintCallable, Category = "Identity")
        FString GetPlayerName() const {
            return PlayerName;
        }

        UFUNCTION(BlueprintCallable, Category = "Identity")
        GenderIdentity GetGender() const {
            return Gender;
        }

        UFUNCTION(BlueprintCallable, Category = "Identity")
        RelationshipType GetRelationship() const {
            return Relationship;
        }

        UFUNCTION(BlueprintCallable, Category = "Identity")
        Religion GetReligion() const {
            return ReligionValue;
        }

        UFUNCTION(BlueprintCallable, Category = "Identity")
        FPersonality GetPersonality() const {
            return PersonalityData;
        }

        UFUNCTION(BlueprintCallable, Category = "Identity")
        void RequestChangeGender(GenderIdentity NewGender, bool bConfirm);

        UFUNCTION(BlueprintCallable, Category = "Identity")
        void RequestChangeReligion(Religion NewReligion, bool bConfirm);

        UFUNCTION(BlueprintCallable, Category = "Identity")
        void RequestChangeRelationship(RelationshipType NewType, bool bConfirm);

        UFUNCTION(BlueprintCallable, Category = "Identity")
        void SetOpenness(float Value);

        UFUNCTION(BlueprintCallable, Category = "Identity")
        void SetConscientiousness(float Value);

        UFUNCTION(BlueprintCallable, Category = "Identity")
        void SetExtraversion(float Value);

        UFUNCTION(BlueprintCallable, Category = "Identity")
        void SetAgreeableness(float Value);

        UFUNCTION(BlueprintCallable, Category = "Identity")
        void SetNeuroticism(float Value);

        DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGenderChanged, GenderIdentity, NewGender);
        DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReligionChanged, Religion, NewReligion);
        DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRelationshipChanged, RelationshipType, NewType);

        UPROPERTY(BlueprintAssignable, Category = "Identity|Events")
        FOnGenderChanged OnGenderChanged;

        UPROPERTY(BlueprintAssignable, Category = "Identity|Events")
        FOnReligionChanged OnReligionChanged;

        UPROPERTY(BlueprintAssignable, Category = "Identity|Events")
        FOnRelationshipChanged OnRelationshipChanged;

        UPROPERTY(BlueprintAssignable, Category = "Identity|Events")
        FOnChangeCancelled OnChangeCancelled;

    private:
        void ShowConfirmationDialog(const FString& Message, EIdentityChangeType ChangeType, bool bConfirm);
        void ApplyGenderChange();
        void ApplyReligionChange();
        void ApplyRelationshipChange();

        UPROPERTY(BlueprintReadWrite, Category = "Identity", meta = (AllowPrivateAccess = "true"))
        FString PlayerName = TEXT("Новый игрок");

        UPROPERTY(BlueprintReadWrite, Category = "Identity", meta = (AllowPrivateAccess = "true"))
        GenderIdentity Gender = GenderIdentity::Other;

        UPROPERTY(BlueprintReadWrite, Category = "Identity", meta = (AllowPrivateAccess = "true"))
        RelationshipType Relationship = RelationshipType::Single;

        UPROPERTY(BlueprintReadWrite, Category = "Identity", meta = (AllowPrivateAccess = "true"))
        Religion ReligionValue = Religion::Other;

        UPROPERTY(BlueprintReadWrite, Category = "Identity", meta = (AllowPrivateAccess = "true"))
        FPersonality PersonalityData;

        UPROPERTY(BlueprintReadWrite, Category = "Identity|State", meta = (AllowPrivateAccess = "true"))
        EIdentityType CurrentIdentity = EIdentityType::None;

        UPROPERTY(BlueprintReadWrite, Category = "Identity|State", meta = (AllowPrivateAccess = "true"))
        int32 IdentityLevel = 0;

        UPROPERTY(BlueprintReadWrite, Category = "Identity|State", meta = (AllowPrivateAccess = "true"))
        float IdentityProgress = 0.0f;

        UPROPERTY(BlueprintReadWrite, Category = "Identity|State", meta = (AllowPrivateAccess = "true"))
        TArray<FString> IdentityHistory;

        UPROPERTY(BlueprintReadWrite, Category = "Identity|State", meta = (AllowPrivateAccess = "true"))
        int32 MaxHistorySize = 50;

        UPROPERTY(BlueprintReadOnly, Category = "Identity|References", meta = (AllowPrivateAccess = "true"))
        AGameManager* GameManager = nullptr;

        UPROPERTY(BlueprintReadOnly, Category = "Identity|References", meta = (AllowPrivateAccess = "true"))
        AVRPlayerController* PlayerController = nullptr;

        UPROPERTY(BlueprintReadOnly, Category = "Identity|References", meta = (AllowPrivateAccess = "true"))
        UChakraSystem* ChakraSystem = nullptr;

        TOptional<GenderIdentity> PendingGender;
        TOptional<Religion> PendingReligion;
        TOptional<RelationshipType> PendingRelationship;
};