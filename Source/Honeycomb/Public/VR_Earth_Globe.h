#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VR_Earth_Globe.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UWidgetComponent;
class UArrowComponent;
class UProceduralMeshComponent;

USTRUCT(BlueprintType)
struct FGlobeDataPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Latitude = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Longitude = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor MarkerColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MarkerSize = 10.0f;
};

UCLASS(Blueprintable, BlueprintType)
class HONEYCOMB_API AVR_Earth_Globe: public AActor
{
    GENERATED_BODY()

    public:
        AVR_Earth_Globe();

    protected:
        virtual void BeginPlay() override;
        virtual void Tick(float DeltaTime) override;

    public:
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Globe")
        float GlobeRadius = 500.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Globe")
        float OrbitSpeed = 0.1f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Globe")
        UTexture2D* WorldTexture = nullptr;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Globe")
        bool bEnableAtmosphere = true;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Globe")
        FLinearColor AtmosphereColor = FLinearColor(0.3f, 0.5f, 1.0f, 0.3f);

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Interaction")
        float RotationSpeed = 2.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Interaction")
        float ZoomSpeed = 0.1f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Interaction")
        float MinScale = 200.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Interaction")
        float MaxScale = 2000.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
        TArray<FGlobeDataPoint> DataPoints;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
        TSubclassOf<AActor> MarkerClass;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
        USceneComponent* Root = nullptr;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
        UStaticMeshComponent* GlobeMesh = nullptr;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
        UStaticMeshComponent* AtmosphereMesh = nullptr;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
        USphereComponent* CollisionSphere = nullptr;

        UFUNCTION(BlueprintCallable, Category = "Globe")
        void LoadDataPoints(const TArray<FGlobeDataPoint>& NewDataPoints);

        UFUNCTION(BlueprintCallable, Category = "Globe")
        void ClearAllMarkers();

        UFUNCTION(BlueprintCallable, Category = "Globe")
        void ResetGlobe();

        UFUNCTION(BlueprintCallable, Category = "Globe")
        void SetGlobeVisibility(bool bVisible);

        UFUNCTION(BlueprintCallable, Category = "Globe")
        void AddDataPoint(float Latitude, float Longitude, const FString& PointName, const FString& Info, FLinearColor Color, float Size = 10.0f);

        UFUNCTION(BlueprintCallable, Category = "Globe")
        void SetAutoRotation(bool bEnabled, float Speed = 5.0f);

        UFUNCTION(BlueprintCallable, Category = "Globe")
        FVector GeoToWorldPosition(float Latitude, float Longitude, float Radius) const;

        UFUNCTION(BlueprintCallable, Category = "Globe")
        void OnMarkerSelect(AActor* MarkerActor);

    protected:
        void CreateGlobe();
        void CreateAtmosphere();
        void SetupVRInteractions();
        void CreateDataMarkers();
        void CreateMarkerActor(const FGlobeDataPoint& Point);

        void HandleRotation(float DeltaTime);
        void HandleZoom(float DeltaTime);
        void UpdateMarkersScale();

        void OnMarkerHover(AActor* MarkerActor);
        void OnMarkerHoverExit(AActor* MarkerActor);
        void HighlightMarker(AActor* Marker, bool bHighlight);

        FGlobeDataPoint* GetDataPointByName(const FString& Name);
        void ShowDataPanel(const FString& PointName, const FString& Information);

        UStaticMesh* GetSphereMesh() const;
        FVector2D GetControllerMovement() const;
        float GetControllerDistance() const;

    private:
        UPROPERTY()
        TArray<AActor*> ActiveMarkers;

        UPROPERTY()
        UMaterialInstanceDynamic* GlobeMaterialInstance = nullptr;

        UPROPERTY()
        UMaterialInstanceDynamic* AtmosphereMaterialInstance = nullptr;

        float OriginalGlobeRadius = 500.0f;
        float CurrentScale = 1.0f;
        bool bIsGrabbed = false;
        bool bIsZooming = false;
        bool bIsRotating = false;
        float AutoRotationSpeed = 5.0f;
        bool bAutoRotate = false;

        FVector LastRightControllerPos = FVector::ZeroVector;
        FVector LastLeftControllerPos = FVector::ZeroVector;
        FVector LastRightControllerRot = FVector::ZeroVector;
        FVector LastLeftControllerRot = FVector::ZeroVector;
};
