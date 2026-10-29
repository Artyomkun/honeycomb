#include "AdaptiveVRPerformance.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UAdaptiveVRPerformance::UAdaptiveVRPerformance()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UAdaptiveVRPerformance::BeginPlay()
{
    Super::BeginPlay();

    OriginalFrameRate = NormalFrameRate;
    TargetFrameRate = NormalFrameRate;

    GetWorld()->GetTimerManager().SetTimer(
        TemperatureCheckTimer,
        this,
        &UAdaptiveVRPerformance::OnTimerTick,
        TemperatureCheckInterval,
        true
    );

    UE_LOG(LogTemp, Log, TEXT("Adaptive VR Performance system initialized"));
}

void UAdaptiveVRPerformance::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TimeSinceLastUpdate += DeltaTime;
    if (TimeSinceLastUpdate >= 0.5f)
    {
        TimeSinceLastUpdate = 0.0f;
        SimulatedLoad = FMath::Clamp(SimulatedLoad + FMath::RandRange(-5.0f, 10.0f), 0.0f, 100.0f);
    }
}

void UAdaptiveVRPerformance::OnTimerTick()
{
    UpdateMetrics();
    ApplyPerformanceSettings();

    if (OnThermalEvent.IsBound())
    {
        OnThermalEvent.Broadcast(CurrentMetrics);
    }
}

void UAdaptiveVRPerformance::UpdateMetrics()
{
    CurrentTemperature = Temperature(TemperatureCheckInterval);

    CurrentMetrics.Temperature = CurrentTemperature;
    CurrentMetrics.WarningLevel = CalculateWarningLevel(CurrentTemperature);
    CurrentMetrics.TargetFrameRate = TargetFrameRate;
}

float UAdaptiveVRPerformance::Temperature(float DeltaTime)
{
    float CpuLoad = FPlatformTime::GetCPUTime().CPUTimePct / 100.0f;
    float GpuLoad = GetGPULoad();
    float LoadFactor = FMath::Max(CpuLoad, GpuLoad);
    float PowerDraw = IdlePower + LoadFactor * (MaxPower - IdlePower);
    float Q_in = PowerDraw * HeatEfficiency;
    float Q_out = HeatTransferCoeff * (CurrentTemperature - AmbientTemperature);
    float dT = (Q_in - Q_out) / HeatCapacity * DeltaTime;
    CurrentTemperature += dT;
    CurrentTemperature = FMath::Clamp(CurrentTemperature, AmbientTemperature, MaxTemperature);
    ApplyThrottling(CurrentTemperature);
    return CurrentTemperature;
}

float UAdaptiveVRPerformance::GetGPULoad() const
{
    return SimulatedLoad;
}

void UAdaptiveVRPerformance::ApplyThrottling(float Temperature)
{
    if (Temperature >= CriticalTemperature)
    {
        bIsThrottling = true;
        SetTargetFrameRate(ThrottlingFrameRate);
    }
}

AdaptiveVRWarningLevel UAdaptiveVRPerformance::CalculateWarningLevel(float Temperature)
{
    if (Temperature >= CriticalTemperature)
    {
        return AdaptiveVRWarningLevel::Throttling;
    }
    else if (Temperature >= WarningTemperature)
    {
        return AdaptiveVRWarningLevel::ThrottlingImminent;
    }
    else
    {
        return AdaptiveVRWarningLevel::NoWarning;
    }
}

void UAdaptiveVRPerformance::ApplyPerformanceSettings()
{
    AdaptiveVRWarningLevel Level = CurrentMetrics.WarningLevel;

    switch (Level)
    {
        case AdaptiveVRWarningLevel::NoWarning:
            TargetFrameRate = NormalFrameRate;
            bIsThrottling = false;
            break;

        case AdaptiveVRWarningLevel::ThrottlingImminent:
            TargetFrameRate = ThrottlingImminentFrameRate;
            bIsThrottling = false;
            break;

        case AdaptiveVRWarningLevel::Throttling:
            TargetFrameRate = ThrottlingFrameRate;
            bIsThrottling = true;
            break;
    }

    SetTargetFrameRate(TargetFrameRate);

    UE_LOG(LogTemp, Log, TEXT("Performance settings applied: Target FPS = %.1f, Warning Level = %d"),
        TargetFrameRate, (int32)Level);
}

void UAdaptiveVRPerformance::SetTargetFrameRate(float FrameRate)
{
    TargetFrameRate = FrameRate;

    if (GEngine)
    {
        GEngine->SetMaxFPS(FrameRate);
        UE_LOG(LogTemp, Log, TEXT("Target frame rate set to: %.1f"), FrameRate);
    }
}

float UAdaptiveVRPerformance::GetTargetFrameRate() const
{
    return TargetFrameRate;
}

AdaptiveVRWarningLevel UAdaptiveVRPerformance::GetWarningLevel() const
{
    return CurrentMetrics.WarningLevel;
}

void UAdaptiveVRPerformance::UpdateThermalStatus()
{
    UpdateMetrics();
    ApplyPerformanceSettings();

    if (OnThermalEvent.IsBound())
    {
        OnThermalEvent.Broadcast(CurrentMetrics);
    }
}

void UAdaptiveVRPerformance::ForcePerformanceMode(AdaptiveVRWarningLevel Level)
{
    CurrentMetrics.WarningLevel = Level;
    ApplyPerformanceSettings();

    if (OnThermalEvent.IsBound())
    {
        OnThermalEvent.Broadcast(CurrentMetrics);
    }

    UE_LOG(LogTemp, Log, TEXT("Performance mode forced to: %d"), (int32)Level);
}

float UAdaptiveVRPerformance::GetCurrentTemperature() const
{
    return CurrentTemperature;
}

bool UAdaptiveVRPerformance::IsThrottling() const
{
    return bIsThrottling;
}
