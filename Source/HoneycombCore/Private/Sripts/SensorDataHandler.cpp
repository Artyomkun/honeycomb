#include "SensorDataHandler.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"

USensorDataHandler::USensorDataHandler()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    TimeSinceLastUpdate = 0.0f;
    SimulationTime = 0.0f;
    bIsSensorReady = false;
}

void USensorDataHandler::BeginPlay()
{
    Super::BeginPlay();
    InitializeSensor();
}

void USensorDataHandler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    TimeSinceLastUpdate += DeltaTime;
    float UpdateInterval = 1.0f / FMath::Clamp(UpdateFrequency, MinUpdateFrequency, MaxUpdateFrequency);
    if (TimeSinceLastUpdate >= UpdateInterval)
    {
        UpdateSensorData();
        TimeSinceLastUpdate = 0.0f;
    }
}

void USensorDataHandler::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ShutdownSensor();
    Super::EndPlay(EndPlayReason);
}

void USensorDataHandler::InitializeSensor()
{
    if (bIsSensorReady)
    {
        LogMessage(TEXT("Sensor already initialized"));
        return;
    }

    SensorBuffer.SetNumUninitialized(SensorDataSize);

    bIsSensorReady = true;
    LogMessage(TEXT("Sensor data handler initialized"));
}

void USensorDataHandler::ShutdownSensor()
{
    CurrentSensorData.Empty();
    SensorBuffer.Empty();
    bIsSensorReady = false;

    LogMessage(TEXT("Sensor data handler shut down"));
}

void USensorDataHandler::UpdateSensorData()
{
    if (!bIsSensorReady)
    {
        return;
    }

    GenerateSensorData();
    SensorBuffer.SetNumUninitialized(CurrentSensorData.Num());
    if (CurrentSensorData.Num() > 0)
    {
        FMemory::Memcpy(SensorBuffer.GetData(), CurrentSensorData.GetData(), CurrentSensorData.Num());
    }

    OnSensorDataUpdated.Broadcast(CurrentSensorData);

    if (bLogOperations)
    {
        LogMessage(FString::Printf(TEXT("Sensor data updated: %d bytes"), CurrentSensorData.Num()));
    }
}

void USensorDataHandler::GenerateSensorData()
{
    if (SensorDataSize <= 0)
    {
        SensorDataSize = 512;
    }

    CurrentSensorData.SetNum(SensorDataSize);
    SimulationTime += 0.016f;

    for (int32 i = 0; i < CurrentSensorData.Num(); i++)
    {
        float Value = FMath::Sin(SimulationTime + i * 0.1f) * 127.0f + 128.0f;
        CurrentSensorData[i] = static_cast<uint8>(FMath::Clamp(Value, 0.0f, 255.0f));
    }
}

void USensorDataHandler::AllocateSensorBuffer()
{
    if (SensorDataSize > 0)
    {
        SensorBuffer.SetNum(SensorDataSize);
    }
}

void USensorDataHandler::FreeSensorBuffer()
{
    SensorBuffer.Empty();
}

void USensorDataHandler::LogMessage(const FString& Message) const
{
    #if WITH_EDITOR || UE_BUILD_DEBUG
        UE_LOG(LogTemp, Log, TEXT("[SensorDataHandler] %s"), *Message);
    #endif
}

void USensorDataHandler::OnHandleAllocated()
{
    LogMessage(TEXT("Sensor data handler ready"));
    OnSensorDataReady.Broadcast();
}

void USensorDataHandler::OnHandleFreed()
{
    LogMessage(TEXT("Sensor data handler cleaned up"));
}