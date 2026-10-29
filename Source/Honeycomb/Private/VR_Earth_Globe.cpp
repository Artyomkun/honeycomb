#include "VR_Earth_Globe.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathUtility.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"

AVR_Earth_Globe::AVR_Earth_Globe()
{
    PrimaryActorTick.bCanEverTick = true;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    GlobeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GlobeMesh"));
    GlobeMesh->SetupAttachment(Root);
    GlobeMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GlobeMesh->SetCollisionObjectType(ECC_WorldDynamic);

    AtmosphereMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AtmosphereMesh"));
    AtmosphereMesh->SetupAttachment(GlobeMesh);
    AtmosphereMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    CollisionSphere->SetupAttachment(GlobeMesh);
    CollisionSphere->SetSphereRadius(GlobeRadius);
}

void AVR_Earth_Globe::BeginPlay()
{
    Super::BeginPlay();

    OriginalGlobeRadius = GlobeRadius;
    CurrentScale = 1.0f;
    bIsGrabbed = false;
    bIsZooming = false;
    bIsRotating = false;
    bAutoRotate = false;
    AutoRotationSpeed = 5.0f;

    CreateGlobe();

    if (GlobeMesh)
    {
        SetupVRInteractions();
        CreateDataMarkers();
    }
}

void AVR_Earth_Globe::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsGrabbed)
    {
        HandleRotation(DeltaTime);
        HandleZoom(DeltaTime);
    }

    if (bAutoRotate && !bIsGrabbed)
    {
        GlobeMesh->AddRelativeRotation(FRotator(0.0f, AutoRotationSpeed * DeltaTime, 0.0f));
    }
}

void AVR_Earth_Globe::CreateGlobe()
{
    if (!GlobeMesh) return;

    UStaticMesh* SphereMesh = GetSphereMesh();
    if (SphereMesh)
    {
        GlobeMesh->SetStaticMesh(SphereMesh);
        GlobeMesh->SetWorldScale3D(FVector(GlobeRadius / 100.0f));
    }

    if (WorldTexture)
    {
        UMaterial* BaseMaterial = LoadObject<UMaterial>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
        if (BaseMaterial)
        {
            GlobeMaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterial, this);
            GlobeMaterialInstance->SetTextureParameterValue(TEXT("BaseTexture"), WorldTexture);
            GlobeMaterialInstance->SetScalarParameterValue(TEXT("Metallic"), 0.0f);
            GlobeMaterialInstance->SetScalarParameterValue(TEXT("Roughness"), 0.8f);
            GlobeMesh->SetMaterial(0, GlobeMaterialInstance);
        }
    }

    if (bEnableAtmosphere)
    {
        CreateAtmosphere();
    }
}

void AVR_Earth_Globe::CreateAtmosphere()
{
    if (!AtmosphereMesh) return;

    UStaticMesh* SphereMesh = GetSphereMesh();
    if (SphereMesh)
    {
        AtmosphereMesh->SetStaticMesh(SphereMesh);
        AtmosphereMesh->SetWorldScale3D(FVector(GlobeRadius * 1.1f / 100.0f));
    }

    UMaterial* BaseMaterial = LoadObject<UMaterial>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
    if (BaseMaterial)
    {
        AtmosphereMaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterial, this);
        AtmosphereMaterialInstance->SetVectorParameterValue(TEXT("Color"), AtmosphereColor);
        AtmosphereMaterialInstance->SetScalarParameterValue(TEXT("Opacity"), AtmosphereColor.A);
        AtmosphereMaterialInstance->SetScalarParameterValue(TEXT("Metallic"), 0.0f);
        AtmosphereMaterialInstance->SetScalarParameterValue(TEXT("Roughness"), 0.0f);
        AtmosphereMesh->SetMaterial(0, AtmosphereMaterialInstance);
    }
}

void AVR_Earth_Globe::SetupVRInteractions()
{
    if (!GlobeMesh) return;

    GlobeMesh->SetSimulatePhysics(true);
    GlobeMesh->SetLinearDamping(2.0f);
    GlobeMesh->SetAngularDamping(2.0f);
    GlobeMesh->SetMassOverrideInKg(NAME_None, 100.0f, true);
    GlobeMesh->SetEnableGravity(false);
    GlobeMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GlobeMesh->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
    GlobeMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
}

void AVR_Earth_Globe::CreateDataMarkers()
{
    ClearAllMarkers();

    for (const FGlobeDataPoint& Point : DataPoints)
    {
        CreateMarkerActor(Point);
    }
}

void AVR_Earth_Globe::CreateMarkerActor(const FGlobeDataPoint& Point)
{
    if (!MarkerClass || !GetWorld()) return;

    FVector WorldPos = GeoToWorldPosition(Point.Latitude, Point.Longitude, GlobeRadius);
    FVector SpawnLocation = GetActorLocation() + WorldPos;

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;

    AActor* Marker = GetWorld()->SpawnActor<AActor>(MarkerClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
    if (Marker)
    {
        Marker->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
        Marker->SetActorScale3D(FVector(Point.MarkerSize / 100.0f));
        ActiveMarkers.Add(Marker);
    }
}

void AVR_Earth_Globe::HandleRotation(float DeltaTime)
{
    if (!GlobeMesh) return;

    static float OrbitAngle = 0.0f;
    OrbitAngle += OrbitSpeed * DeltaTime;
    
    float OrbitRadius = 1000.0f;
    
    float X = OrbitRadius * FMath::Cos(OrbitAngle);
    float Z = OrbitRadius * FMath::Sin(OrbitAngle);
    SetActorLocation(FVector(X, 0.0f, Z));

    GlobeMesh->AddLocalRotation(FRotator(0.0f, RotationSpeed * DeltaTime, 0.0f));

    GlobeMesh->SetRelativeRotation(FRotator(23.5f, 0.0f, 0.0f));
}

void AVR_Earth_Globe::HandleZoom(float DeltaTime)
{
    float Distance = GetControllerDistance();
    if (Distance > 0.0f)
    {
        float TargetRadius = OriginalGlobeRadius * (1.0f + Distance * 0.01f);
        GlobeRadius = FMath::FInterpTo(GlobeRadius, TargetRadius, DeltaTime, 5.0f);
        GlobeMesh->SetWorldScale3D(FVector(GlobeRadius / 100.0f));
        UpdateMarkersScale();
    }
}

void AVR_Earth_Globe::UpdateMarkersScale()
{
    float BaseScale = (OriginalGlobeRadius / GlobeRadius) * 0.1f;

    for (AActor* Marker : ActiveMarkers)
    {
        if (Marker)
        {
            FString Name = Marker->GetName().Replace(TEXT("_Marker"), TEXT(""));
            FGlobeDataPoint* Point = GetDataPointByName(Name);
            if (Point)
            {
                FVector NewPos = GeoToWorldPosition(Point->Latitude, Point->Longitude, GlobeRadius);
                Marker->SetActorRelativeLocation(NewPos);
                Marker->SetActorScale3D(FVector(Point->MarkerSize * BaseScale));
            }
        }
    }
}

void AVR_Earth_Globe::OnMarkerHover(AActor* MarkerActor)
{
    HighlightMarker(MarkerActor, true);
}

void AVR_Earth_Globe::OnMarkerHoverExit(AActor* MarkerActor)
{
    HighlightMarker(MarkerActor, false);
}

void AVR_Earth_Globe::OnMarkerSelect(AActor* MarkerActor)
{
    if (!MarkerActor) return;

    FString PointName = MarkerActor->GetName().Replace(TEXT("_Marker"), TEXT(""));
    FGlobeDataPoint* DataPoint = GetDataPointByName(PointName);

    if (DataPoint)
    {
        ShowDataPanel(DataPoint->Name, DataPoint->Description);
    }
}

void AVR_Earth_Globe::HighlightMarker(AActor* Marker, bool bHighlight)
{
    if (!Marker) return;

    FVector Scale = Marker->GetActorScale3D();
    if (bHighlight)
    {
        Marker->SetActorScale3D(Scale * 1.2f);
    }
    else
    {
        Marker->SetActorScale3D(Scale / 1.2f);
    }
}

FGlobeDataPoint* AVR_Earth_Globe::GetDataPointByName(const FString& Name)
{
    for (FGlobeDataPoint& Point : DataPoints)
    {
        if (Point.Name == Name)
        {
            return &Point;
        }
    }
    return nullptr;
}

void AVR_Earth_Globe::ShowDataPanel(const FString& PointName, const FString& Information)
{
    UE_LOG(LogTemp, Log, TEXT("=== %s ===\n%s"), *PointName, *Information);
}

FVector AVR_Earth_Globe::GeoToWorldPosition(float Latitude, float Longitude, float Radius) const
{
    float LatRad = FMath::DegreesToRadians(Latitude);
    float LonRad = FMath::DegreesToRadians(-Longitude);

    float X = Radius * FMath::Cos(LatRad) * FMath::Cos(LonRad);
    float Z = Radius * FMath::Cos(LatRad) * FMath::Sin(LonRad);
    float Y = Radius * FMath::Sin(LatRad);

    return FVector(X, Y, Z);
}

UStaticMesh* AVR_Earth_Globe::GetSphereMesh() const
{
    return LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
}

void AVR_Earth_Globe::LoadDataPoints(const TArray<FGlobeDataPoint>& NewDataPoints)
{
    DataPoints.Empty();
    DataPoints.Append(NewDataPoints);
    CreateDataMarkers();
}

void AVR_Earth_Globe::ClearAllMarkers()
{
    for (AActor* Marker : ActiveMarkers)
    {
        if (Marker) Marker->Destroy();
    }
    ActiveMarkers.Empty();
}

void AVR_Earth_Globe::ResetGlobe()
{
    if (GlobeMesh)
    {
        GlobeMesh->SetWorldRotation(FRotator::ZeroRotator);
        GlobeRadius = OriginalGlobeRadius;
        GlobeMesh->SetWorldScale3D(FVector(GlobeRadius / 100.0f));
        UpdateMarkersScale();
    }
}

void AVR_Earth_Globe::SetGlobeVisibility(bool bVisible)
{
    if (GlobeMesh) GlobeMesh->SetVisibility(bVisible);
    if (AtmosphereMesh) AtmosphereMesh->SetVisibility(bVisible);
}

void AVR_Earth_Globe::AddDataPoint(float Latitude, float Longitude, const FString& PointName, const FString& Info, FLinearColor Color, float Size)
{
    FGlobeDataPoint NewPoint;
    NewPoint.Latitude = Latitude;
    NewPoint.Longitude = Longitude;
    NewPoint.Name = PointName;
    NewPoint.Description = Info;
    NewPoint.MarkerColor = Color;
    NewPoint.MarkerSize = Size;

    DataPoints.Add(NewPoint);

    if (MarkerClass && GetWorld())
    {
        CreateMarkerActor(NewPoint);
    }
}

void AVR_Earth_Globe::SetAutoRotation(bool bEnabled, float Speed)
{
    bAutoRotate = bEnabled;
    AutoRotationSpeed = Speed;
}

FVector2D AVR_Earth_Globe::GetControllerMovement() const
{
    return FVector2D::ZeroVector;
}

float AVR_Earth_Globe::GetControllerDistance() const
{
    return 0.0f;
}
