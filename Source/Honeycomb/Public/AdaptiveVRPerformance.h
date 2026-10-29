#pragma once

#include "HoneycombCore.h"
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AdaptiveVRPerformance.generated.h"

UENUM(BlueprintType)
enum class AdaptiveVRWarningLevel: uint8
{
    NoWarning UMETA(DisplayName = "No Warning"),
    ThrottlingImminent UMETA(DisplayName = "Throttling Imminent"),
    Throttling UMETA(DisplayName = "Throttling")
};

USTRUCT(BlueprintType)
struct FAdaptiveVRMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Temperature = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AdaptiveVRWarningLevel WarningLevel = AdaptiveVRWarningLevel::NoWarning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TargetFrameRate = 90.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnThermalEvent, const FAdaptiveVRMetrics&, Metrics);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HONEYCOMB_API UAdaptiveVRPerformance: public UActorComponent
{
    GENERATED_BODY()

    public:
        UAdaptiveVRPerformance();

    protected:
        virtual void BeginPlay() override;
        virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    public:
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Adaptive VR")
        FAdaptiveVRMetrics CurrentMetrics;

        UPROPERTY(BlueprintAssignable, Category = "Adaptive VR")
        FOnThermalEvent OnThermalEvent;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive VR|Settings")
        float NormalFrameRate = 90.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive VR|Settings")
        float ThrottlingImminentFrameRate = 60.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive VR|Settings")
        float ThrottlingFrameRate = 45.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive VR|Settings")
        float TemperatureCheckInterval = 1.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive VR|Settings")
        float CriticalTemperature = 80.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive VR|Settings")
        float WarningTemperature = 65.0f;

        UFUNCTION(BlueprintCallable, Category = "Adaptive VR")
        void SetTargetFrameRate(float FrameRate);

        UFUNCTION(BlueprintCallable, Category = "Adaptive VR")
        float GetTargetFrameRate() const;

        UFUNCTION(BlueprintCallable, Category = "Adaptive VR")
        AdaptiveVRWarningLevel GetWarningLevel() const;

        UFUNCTION(BlueprintCallable, Category = "Adaptive VR")
        void UpdateThermalStatus();

        UFUNCTION(BlueprintCallable, Category = "Adaptive VR")
        void ForcePerformanceMode(AdaptiveVRWarningLevel Level);

        UFUNCTION(BlueprintCallable, Category = "Adaptive VR")
        float GetCurrentTemperature() const;

        UFUNCTION(BlueprintCallable, Category = "Adaptive VR")
        bool IsThrottling() const;

    protected:
        void UpdateMetrics();
        void ApplyPerformanceSettings();

        float Temperature(float DeltaTime);
        float GetGPULoad() const;
        void ApplyThrottling(float Temperature);
        AdaptiveVRWarningLevel CalculateWarningLevel(float Temperature);

        UFUNCTION()
        void OnTimerTick();

    private:
        FTimerHandle TemperatureCheckTimer;

        float CurrentTemperature = 35.0f;
        float TargetFrameRate = 90.0f;
        bool bIsThrottling = false;

        float SimulatedLoad = 0.0f;
        float TimeSinceLastUpdate = 0.0f;

        float OriginalFrameRate = 90.0f;

        float IdlePower = 5.0f;
        float MaxPower = 15.0f;
        float HeatEfficiency = 0.8f;
        float HeatTransferCoeff = 0.1f;
        float AmbientTemperature = 25.0f;
        float HeatCapacity = 500.0f;
        float MaxTemperature = 100.0f;
};
