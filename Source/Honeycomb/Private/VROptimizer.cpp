#include "VROptimizer.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameUserSettings.h"
#include "Engine/GameEngine.h"
#include "Engine/World.h"

UVROptimizer::UVROptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    CurrentFPS = 0.0f;
    TimeSinceLastFPSUpdate = 0.0f;
    FrameCounter = 0;
    CurrentRenderScale = 1.0f;

    DefaultTargetFrameRate = TargetFrameRate;
    DefaultRenderScale = RenderScale;
    DefaultVSyncCount = VSyncCount;
}

void UVROptimizer::BeginPlay()
{
    Super::BeginPlay();

    ApplyVRSettings();

    UE_LOG(LogTemp, Log, TEXT("VR Optimizer initialized: Target FPS=%d, Render Scale=%.2f, VSync=%d"), TargetFrameRate, RenderScale, VSyncCount);
}

void UVROptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateFPSStats(DeltaTime);

    if (bEnableDynamicQuality)
    {
        UpdateDynamicQuality(DeltaTime);
    }
}

void UVROptimizer::ApplyVRSettings()
{
    SetTargetFrameRate(TargetFrameRate);
    SetRenderScale(RenderScale);
    SetVSync(VSyncCount == 0);
}

void UVROptimizer::SetTargetFrameRate(int32 NewFrameRate)
{
    TargetFrameRate = NewFrameRate;

    if (GEngine)
    {
        GEngine->SetMaxFPS((float)NewFrameRate);
        UE_LOG(LogTemp, Log, TEXT("Target frame rate set to: %d"), NewFrameRate);
    }

    if (UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings())
    {
        Settings->SetFrameRateLimit((float)NewFrameRate);
        Settings->ApplySettings(false);
    }
}

void UVROptimizer::SetRenderScale(float NewRenderScale)
{
    RenderScale = FMath::Clamp(NewRenderScale, MinRenderScale, MaxRenderScale);
    CurrentRenderScale = RenderScale;

    if (UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings())
    {
        int32 ScreenPercentage = FMath::RoundToInt(RenderScale * 100.0f);
        Settings->SetResolutionScaleValueEx(ScreenPercentage);
        Settings->ApplySettings(false);

        UE_LOG(LogTemp, Log, TEXT("Render scale set to: %.2f (ScreenPercentage: %d%%)"), RenderScale, ScreenPercentage);
    }
}

void UVROptimizer::SetVSync(bool bEnabled)
{
    VSyncCount = bEnabled ? 1 : 0;

    if (UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings())
    {
        Settings->SetVSyncEnabled(bEnabled);
        Settings->ApplySettings(false);

        UE_LOG(LogTemp, Log, TEXT("VSync %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
    }
}

float UVROptimizer::GetCurrentFPS() const
{
    return CurrentFPS;
}

bool UVROptimizer::IsPerformanceOK() const
{
    return CurrentFPS >= LowFpsThreshold;
}

void UVROptimizer::ResetToDefault()
{
    TargetFrameRate = DefaultTargetFrameRate;
    RenderScale = DefaultRenderScale;
    VSyncCount = DefaultVSyncCount;

    ApplyVRSettings();

    UE_LOG(LogTemp, Log, TEXT("VR Optimizer reset to default settings"));
}

void UVROptimizer::UpdateFPSStats(float DeltaTime)
{
    TimeSinceLastFPSUpdate += DeltaTime;
    FrameCounter++;

    if (TimeSinceLastFPSUpdate >= 1.0f)
    {
        CurrentFPS = FrameCounter / TimeSinceLastFPSUpdate;
        TimeSinceLastFPSUpdate = 0.0f;
        FrameCounter = 0;

        static float LastLoggedFPS = 0.0f;
        if (FMath::Abs(CurrentFPS - LastLoggedFPS) > 5.0f)
        {
            UE_LOG(LogTemp, Verbose, TEXT("Current FPS: %.1f"), CurrentFPS);
            LastLoggedFPS = CurrentFPS;
        }
    }
}

void UVROptimizer::UpdateDynamicQuality(float DeltaTime)
{
    if (CurrentFPS < LowFpsThreshold && CurrentFPS > 0.0f)
    {
        float NewRenderScale = FMath::Max(CurrentRenderScale - QualityAdjustmentSpeed * DeltaTime, MinRenderScale);
        if (NewRenderScale != CurrentRenderScale)
        {
            SetRenderScale(NewRenderScale);
            UE_LOG(LogTemp, Warning, TEXT("Performance low (%.1f FPS), reducing render scale to %.2f"),
                CurrentFPS, NewRenderScale);
        }
    }
    else if (CurrentFPS > LowFpsThreshold + 15.0f)
    {
        float NewRenderScale = FMath::Min(CurrentRenderScale + QualityAdjustmentSpeed * DeltaTime * 0.5f, MaxRenderScale);
        if (NewRenderScale != CurrentRenderScale)
        {
            SetRenderScale(NewRenderScale);
            UE_LOG(LogTemp, Log, TEXT("Performance good (%.1f FPS), restoring render scale to %.2f"), CurrentFPS, NewRenderScale);
        }
    }
}
