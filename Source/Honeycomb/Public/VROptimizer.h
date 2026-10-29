#pragma once

#include "HoneycombCore.h"
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VROptimizer.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HONEYCOMB_API UVROptimizer: public UActorComponent
{
    GENERATED_BODY()

    public:
        UVROptimizer();

    protected:
        virtual void BeginPlay() override;
        virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    public:
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Optimizer")
        int32 TargetFrameRate = 90;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Optimizer")
        float RenderScale = 1.2f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Optimizer")
        int32 VSyncCount = 0;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Optimizer|Dynamic")
        bool bEnableDynamicQuality = true;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Optimizer|Dynamic")
        float LowFpsThreshold = 80.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Optimizer|Dynamic")
        float MinRenderScale = 1.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Optimizer|Dynamic")
        float MaxRenderScale = 1.5f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Optimizer|Dynamic")
        float QualityAdjustmentSpeed = 0.1f;

        UFUNCTION(BlueprintCallable, Category = "VR Optimizer")
        void SetTargetFrameRate(int32 NewFrameRate);

        UFUNCTION(BlueprintCallable, Category = "VR Optimizer")
        void SetRenderScale(float NewRenderScale);

        UFUNCTION(BlueprintCallable, Category = "VR Optimizer")
        void SetVSync(bool bEnabled);

        UFUNCTION(BlueprintCallable, Category = "VR Optimizer")
        float GetCurrentFPS() const;

        UFUNCTION(BlueprintCallable, Category = "VR Optimizer")
        bool IsPerformanceOK() const;

        UFUNCTION(BlueprintCallable, Category = "VR Optimizer")
        void ResetToDefault();

    protected:
        void ApplyVRSettings();
        void UpdateDynamicQuality(float DeltaTime);
        void UpdateFPSStats(float DeltaTime);

    private:
        float CurrentFPS = 0.0f;
        float FPSCounter = 0.0f;
        int32 FrameCounter = 0;
        float TimeSinceLastFPSUpdate = 0.0f;

        float CurrentRenderScale = 1.2f;

        int32 DefaultTargetFrameRate = 90;
        float DefaultRenderScale = 1.2f;
        int32 DefaultVSyncCount = 0;

        float LastFrameTime = 0.0f;
};
