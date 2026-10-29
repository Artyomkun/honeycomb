#pragma once

#include "HoneycombCore.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VRUIController.generated.h"

class UWidgetComponent;
class UUserWidget;
class UCameraComponent;

UCLASS(Blueprintable, BlueprintType)
class HONEYCOMB_API AVRUIController: public AActor
{
    GENERATED_BODY()

    public:
        AVRUIController();

    protected:
        virtual void BeginPlay() override;
        virtual void Tick(float DeltaTime) override;

    public:
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR UI")
        UWidgetComponent* VRWidgetComponent;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR UI")
        UCameraComponent* PlayerCamera;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR UI")
        float UIDistance = 200.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR UI")
        TSubclassOf<UUserWidget> WidgetClass;

        UPROPERTY()
        UUserWidget* CurrentWidget;

        UFUNCTION(BlueprintCallable, Category = "VR UI")
        void UpdateUIPosition();

        UFUNCTION(BlueprintCallable, Category = "VR UI")
        void ShowUI(bool bShow);

        UFUNCTION(BlueprintCallable, Category = "VR UI")
        void ToggleUI();

        UFUNCTION(BlueprintCallable, Category = "VR UI|Buttons")
        void OnSatelliteButtonPressed();

        UFUNCTION(BlueprintCallable, Category = "VR UI|Buttons")
        void OnTemperatureDataButtonPressed();

        UFUNCTION(BlueprintCallable, Category = "VR UI|Slider")
        void OnTimeSliderChanged(float Value);

        UFUNCTION(BlueprintImplementableEvent, Category = "VR UI|Events")
        void OnSatelliteViewActivated();

        UFUNCTION(BlueprintImplementableEvent, Category = "VR UI|Events")
        void OnTemperatureViewActivated();

        UFUNCTION(BlueprintImplementableEvent, Category = "VR UI|Events")
        void OnTimeChanged(float Value);

    protected:
        void CreateVRWidget();
        void PositionUI();

    private:
        bool bIsUIVisible = true;
        float TimeValue = 0.5f;
};