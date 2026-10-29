#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Geo/GeoTypes.h"
#include "ProceduralStreet.generated.h"

class UMaterialInterface;
class UProceduralMeshComponent;

/**
 * Runtime street made of OpenStreetMap data.
 *
 * Holds two procedural mesh components: one for extruded building footprints and
 * one for flat road ribbons. Geometry arrives already projected to street-local
 * centimetres (X = east, Y = north, Z = up), usually from UGeoStreetSubsystem.
 * Buildings become one mesh section each so they can be picked and tinted
 * independently; roads share a single section.
 */
UCLASS(Blueprintable, BlueprintType)
class HONEYCOMB_API AProceduralStreet : public AActor
{
    GENERATED_BODY()

public:
    AProceduralStreet();

    /** Rebuilds all geometry from projected OSM data. Clears anything existing. */
    UFUNCTION(BlueprintCallable, Category = "Honeycomb|Street")
    void BuildStreet(const TArray<FGeoBuilding>& Buildings, const TArray<FGeoRoad>& Roads);

    UFUNCTION(BlueprintCallable, Category = "Honeycomb|Street")
    void ClearStreet();

    UFUNCTION(BlueprintPure, Category = "Honeycomb|Street")
    int32 GetBuildingCount() const { return BuildingCount; }

    UFUNCTION(BlueprintPure, Category = "Honeycomb|Street")
    int32 GetRoadCount() const { return RoadCount; }

    UFUNCTION(BlueprintPure, Category = "Honeycomb|Street")
    int32 GetTriangleCount() const { return TriangleCount; }

    /** Extrudes one footprint into walls + roof. Returns triangle count added. */
    static int32 BuildBuildingGeometry(
        const TArray<FVector2D>& FootprintLocalCm,
        float HeightCm,
        TArray<FVector>& OutVerts,
        TArray<int32>& OutTris,
        TArray<FVector>& OutNormals);

    /** Turns a polyline into a flat ribbon. Returns triangle count added. */
    static int32 BuildRoadGeometry(
        const TArray<FVector2D>& CenterlineLocalCm,
        float WidthCm,
        float Zcm,
        TArray<FVector>& OutVerts,
        TArray<int32>& OutTris,
        TArray<FVector>& OutNormals);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Honeycomb|Street")
    UProceduralMeshComponent* BuildingsMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Honeycomb|Street")
    UProceduralMeshComponent* RoadsMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Honeycomb|Street|Look")
    UMaterialInterface* BuildingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Honeycomb|Street|Look")
    UMaterialInterface* RoadMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Honeycomb|Street|Collision")
    bool bCreateCollision = true;

    /** Roads float this far above the ground plane to avoid z-fighting, in cm. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Honeycomb|Street|Look")
    float RoadHeightCm = 6.0f;

private:
    int32 BuildingCount = 0;
    int32 RoadCount = 0;
    int32 TriangleCount = 0;
};
