#pragma once

#include "HoneycombCore.h"
#include "CoreMinimal.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "UObject/Object.h"
#include "DimensionSystem.generated.h"

class UUIDimensionDisplay;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhysicsChanged, int32, Index);

USTRUCT(BlueprintType)
struct FDimensionData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    int32 Index = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 Level = 0;

    UPROPERTY(BlueprintReadWrite)
    FString Name;

    UPROPERTY(BlueprintReadWrite)
    FString Description;

    UPROPERTY(BlueprintReadWrite)
    FLinearColor DimensionColor = FLinearColor::White;

    UPROPERTY(BlueprintReadWrite)
    FLinearColor WorldColor = FLinearColor::White;

    UPROPERTY(BlueprintReadWrite)
    float RealityShift = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float GravityScale = 1.0f;

    UPROPERTY(BlueprintReadWrite)
    float TimeDilation = 1.0f;

    UPROPERTY(BlueprintReadWrite)
    TArray<FString> Features;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UPhysicalMaterial* PhysicsMaterial = nullptr;

    UPROPERTY(BlueprintReadWrite)
    bool bIsUnlocked = false;

    UPROPERTY(BlueprintReadWrite)
    int32 RequiredChakraLevel = 1;
};

UCLASS(BlueprintType, Blueprintable)
class HONEYCOMB_API UDimensionSystem: public UObject
{
    GENERATED_BODY()

    public:
        UDimensionSystem();

        void InitializeDimensions();
        void OnDimensionChange(int32 NewDimensionLevel);
        bool IsDimensionUnlocked(int32 Level) const;
        FDimensionData GetDimensionData(int32 Level) const;

        float GetRealityStability() const {
            return RealityStability;
        }

        float GetDimensionalShift() const {
            return DimensionalShift;
        }
        
        void ModifyRealityStability(float Amount);
        void TriggerRealityShift();
        void StabilizeReality();
        void ActivateShift(int32 NewDimensionLevel);

        TArray<FDimensionData> GetAllDimensions() const {
            return Dimensions;
        }

    private:
        void ApplyDimensionEffects(const FDimensionData& Dimension);
        void ApplyPhysicsChanges(const FDimensionData& Dimension);
        void ApplyPhysicsMaterials(class UPhysicalMaterial* Material);
        void UpdateUI(const FDimensionData& Dimension);

        UPROPERTY()
        TArray<FDimensionData> Dimensions;

        UPROPERTY()
        float RealityStability = 100.0f;

        UPROPERTY()
        float DimensionalShift = 0.0f;

        UPROPERTY(BlueprintAssignable, Category = "DimensionSystem")
        FOnPhysicsChanged OnPhysicsChanged;

        class AGameManager* GameManagerRef = nullptr;
        class UUIDimensionDisplay* DimensionUI = nullptr;
        int32 CurrentDimensionLevel = 1;
        int32 CurrentDimensionIndex = 1;
};
