#pragma once

#include "HoneycombCore.h"
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "AtlantisLoreSystem.generated.h"

class AGameManager;
class AVRUIManager;
class UTexture2D;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAtlantisMemoryTriggered);

USTRUCT(BlueprintType)
struct HONEYCOMB_API FAtlantisMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* Image = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USoundBase* Sound = nullptr;
};

UCLASS(BlueprintType, Blueprintable)
class HONEYCOMB_API UAtlantisLoreSystem: public UObject
{
    GENERATED_BODY()

    public:
        UAtlantisLoreSystem();

        UFUNCTION(BlueprintCallable, Category = "AtlantisLore")
        void TriggerMemory();

        UFUNCTION(BlueprintCallable, Category = "AtlantisLore")
        FAtlantisMemory GetAtlantisMemory() const;

        UFUNCTION(BlueprintCallable, Category = "AtlantisLore")
        bool GetRemembersAtlantis() const { return bRemembersAtlantis; }

        UFUNCTION(BlueprintCallable, Category = "AtlantisLore")
        bool GetFoundAtlanteanRuins() const { return bFoundAtlanteanRuins; }

        UFUNCTION(BlueprintCallable, Category = "AtlantisLore")
        bool GetActivatedCrystalGrid() const { return bActivatedCrystalGrid; }

        UFUNCTION(BlueprintCallable, Category = "AtlantisLore")
        void SetFoundAtlanteanRuins(bool bFound) { bFoundAtlanteanRuins = bFound; }

        UFUNCTION(BlueprintCallable, Category = "AtlantisLore")
        void SetActivatedCrystalGrid(bool bActivated) { bActivatedCrystalGrid = bActivated; }

        UPROPERTY(BlueprintAssignable, Category = "AtlantisLore")
        FOnAtlantisMemoryTriggered OnAtlantisMemoryTriggered;

    private:
        UPROPERTY(BlueprintReadWrite, Category = "AtlantisLore", meta = (AllowPrivateAccess = "true"))
        bool bRemembersAtlantis = false;

        UPROPERTY(BlueprintReadWrite, Category = "AtlantisLore", meta = (AllowPrivateAccess = "true"))
        bool bFoundAtlanteanRuins = false;

        UPROPERTY(BlueprintReadWrite, Category = "AtlantisLore", meta = (AllowPrivateAccess = "true"))
        bool bActivatedCrystalGrid = false;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AtlantisLore|Data", meta = (AllowPrivateAccess = "true"))
        TArray<FAtlantisMemory> AtlantisMemories;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AtlantisLore|Data", meta = (AllowPrivateAccess = "true"))
        USoundBase* AtlantisSound = nullptr;

        UPROPERTY(BlueprintReadOnly, Category = "AtlantisLore|References", meta = (AllowPrivateAccess = "true"))
        AVRUIManager* VRUIManager = nullptr;

        UPROPERTY(BlueprintReadOnly, Category = "AtlantisLore|References", meta = (AllowPrivateAccess = "true"))
        AGameManager* GameManager = nullptr;
};