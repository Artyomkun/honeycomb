#include "VRUIController.h"
#include "Components/WidgetComponent.h"
#include "Camera/CameraComponent.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

AVRUIController::AVRUIController()
{
    PrimaryActorTick.bCanEverTick = true;

    USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    SetRootComponent(Root);

    VRWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("VRWidgetComponent"));
    VRWidgetComponent->SetupAttachment(RootComponent);
    VRWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
    VRWidgetComponent->SetDrawSize(FVector2D(800.0f, 600.0f));
    VRWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    UIDistance = 200.0f;
    TimeValue = 0.5f;
    bIsUIVisible = true;
}

void AVRUIController::BeginPlay()
{
    Super::BeginPlay();

    if (!PlayerCamera)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC)
        {
            APawn* Pawn = PC->GetPawn();
            if (Pawn)
            {
                PlayerCamera = Cast<UCameraComponent>(Pawn->FindComponentByClass<UCameraComponent>());
            }
        }
    }

    if (!PlayerCamera)
    {
        UE_LOG(LogTemp, Warning, TEXT("AVRUIController: PlayerCamera not found!"));
    }

    if (WidgetClass)
    {
        CreateVRWidget();
    }

    PositionUI();
}

void AVRUIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (VRWidgetComponent && PlayerCamera)
    {
        PositionUI();
    }
}

void AVRUIController::CreateVRWidget()
{
    if (!WidgetClass || !GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("AVRUIController: WidgetClass or World is null!"));
        return;
    }

    CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), WidgetClass);
    if (CurrentWidget)
    {
        VRWidgetComponent->SetWidget(CurrentWidget);
        VRWidgetComponent->SetTwoSided(true);

        UE_LOG(LogTemp, Log, TEXT("VR UIController: Widget created successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AVRUIController: Failed to create widget!"));
    }
}

void AVRUIController::PositionUI()
{
    if (!VRWidgetComponent || !PlayerCamera) return;

    FVector CameraLocation = PlayerCamera->GetComponentLocation();
    FRotator CameraRotation = PlayerCamera->GetComponentRotation();
    FVector ForwardVector = CameraRotation.Vector();
    FVector UILocation = CameraLocation + ForwardVector * UIDistance;
    FVector LookAtDirection = (UILocation - CameraLocation).GetSafeNormal();
    FRotator LookAtRotation = LookAtDirection.Rotation();

    VRWidgetComponent->SetWorldLocation(UILocation);
    VRWidgetComponent->SetWorldRotation(LookAtRotation);

    if (!VRWidgetComponent->IsVisible() && bIsUIVisible)
    {
        VRWidgetComponent->SetVisibility(true);
    }
}

void AVRUIController::UpdateUIPosition()
{
    PositionUI();
}

void AVRUIController::ShowUI(bool bShow)
{
    bIsUIVisible = bShow;
    if (VRWidgetComponent)
    {
        VRWidgetComponent->SetVisibility(bShow);
    }
}

void AVRUIController::ToggleUI()
{
    ShowUI(!bIsUIVisible);
}

void AVRUIController::OnSatelliteButtonPressed()
{
    UE_LOG(LogTemp, Log, TEXT("Switching to satellite view in VR"));

    OnSatelliteViewActivated();
}

void AVRUIController::OnTemperatureDataButtonPressed()
{
    UE_LOG(LogTemp, Log, TEXT("Showing temperature data in VR"));

    OnTemperatureViewActivated();
}

void AVRUIController::OnTimeSliderChanged(float Value)
{
    TimeValue = FMath::Clamp(Value, 0.0f, 1.0f);
    UE_LOG(LogTemp, Verbose, TEXT("Time changed to: %.2f"), TimeValue);

    OnTimeChanged(TimeValue);
}
