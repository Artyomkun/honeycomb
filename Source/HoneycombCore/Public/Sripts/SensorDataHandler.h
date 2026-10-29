#pragma once

#include "HoneycombCore.h"
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SensorDataHandler.generated.h"

#ifndef HONEYCOMCORE_API
    #define HONEYCOMCORE_API
#endif

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSensorDataReady);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSensorDataUpdated, const TArray<uint8>&, Data);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HONEYCOMCORE_API USensorDataHandler : public UActorComponent
{
    GENERATED_BODY()

    public:
        USensorDataHandler();

    protected:
        virtual void BeginPlay() override;
        virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
        virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    public:
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensor")
        int32 SensorDataSize = 512;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensor")
        float UpdateFrequency = 30.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensor")
        bool bLogOperations = false;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensor")
        float MinUpdateFrequency = 1.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sensor")
        float MaxUpdateFrequency = 1000.0f;

        UPROPERTY(BlueprintAssignable, Category = "Sensor|Events")
        FOnSensorDataReady OnSensorDataReady;

        UPROPERTY(BlueprintAssignable, Category = "Sensor|Events")
        FOnSensorDataUpdated OnSensorDataUpdated;

        UFUNCTION(BlueprintCallable, Category = "Sensor")
        void InitializeSensor();

        UFUNCTION(BlueprintCallable, Category = "Sensor")
        void ShutdownSensor();

        UFUNCTION(BlueprintCallable, Category = "Sensor")
        void UpdateSensorData();

        UFUNCTION(BlueprintCallable, Category = "Sensor")
        const TArray<uint8>& GetCurrentSensorData() const { return CurrentSensorData; }

        UFUNCTION(BlueprintCallable, Category = "Sensor")
        bool IsSensorReady() const { return bIsSensorReady; }

    protected:
        void AllocateSensorBuffer();
        void FreeSensorBuffer();
        void GenerateSensorData();
        void LogMessage(const FString& Message) const;

        UFUNCTION()
        void OnHandleAllocated();

        UFUNCTION()
        void OnHandleFreed();

    private:
        TArray<uint8> CurrentSensorData;
        TArray<uint8> SensorBuffer;

        bool bIsSensorReady = false;
        float TimeSinceLastUpdate = 0.0f;
        float SimulationTime = 0.0f;
};