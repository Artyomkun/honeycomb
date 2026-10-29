#include "Geo/ProceduralStreet.h"

#include "Algo/Reverse.h"
#include "Materials/MaterialInterface.h"
#include "ProceduralMeshComponent.h"

namespace ProceduralStreetPrivate
{
    const TArray<FLinearColor> BuildingColors;
    const TArray<FProcMeshTangent> NoTangents;

    /** Makes a simple planar UV from local cm, tiling once per 4 m. */
    FVector2D PlanarUV(const FVector& P)
    {
        return FVector2D(P.X / 400.0f, P.Y / 400.0f);
    }
}

AProceduralStreet::AProceduralStreet()
{
    PrimaryActorTick.bCanEverTick = false;

    BuildingsMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("BuildingsMesh"));
    SetRootComponent(BuildingsMesh);
    BuildingsMesh->bUseAsyncCooking = true;
    BuildingsMesh->bUseComplexAsSimpleCollision = false;
    BuildingsMesh->SetCollisionObjectType(ECC_WorldStatic);
    BuildingsMesh->SetCollisionResponseToAllChannels(ECR_Block);
    BuildingsMesh->SetGenerateOverlapEvents(false);
    BuildingsMesh->SetCanEverAffectNavigation(true);
    BuildingsMesh->SetCastShadow(true);

    RoadsMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("RoadsMesh"));
    RoadsMesh->SetupAttachment(BuildingsMesh);
    RoadsMesh->bUseAsyncCooking = true;
    RoadsMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RoadsMesh->SetGenerateOverlapEvents(false);
    RoadsMesh->SetCastShadow(false);
}

int32 AProceduralStreet::BuildBuildingGeometry(
    const TArray<FVector2D>& FootprintLocalCm,
    float HeightCm,
    TArray<FVector>& OutVerts,
    TArray<int32>& OutTris,
    TArray<FVector>& OutNormals)
{
    const int32 Count = FootprintLocalCm.Num();
    if (Count < 3 || HeightCm <= 0.0f)
    {
        return 0;
    }

    // Normalise winding to counter-clockwise so wall normals point outwards.
    TArray<FVector2D> Points = FootprintLocalCm;
    float SignedArea = 0.0f;
    for (int32 i = 0; i < Count; ++i)
    {
        const FVector2D& A = Points[i];
        const FVector2D& B = Points[(i + 1) % Count];
        SignedArea += A.X * B.Y - B.X * A.Y;
    }
    if (SignedArea < 0.0f)
    {
        Algo::Reverse(Points);
    }

    const int32 BaseIndex = OutVerts.Num();
    const float H = HeightCm;

    // Walls: two triangles per footprint edge.
    for (int32 i = 0; i < Count; ++i)
    {
        const FVector2D& A = Points[i];
        const FVector2D& B = Points[(i + 1) % Count];

        FVector2D Edge = B - A;
        Edge.Normalize();
        const FVector Normal(Edge.Y, -Edge.X, 0.0f);

        const int32 I0 = OutVerts.Num();
        OutVerts.Add(FVector(A.X, A.Y, 0.0f));
        OutVerts.Add(FVector(B.X, B.Y, 0.0f));
        OutVerts.Add(FVector(B.X, B.Y, H));
        OutVerts.Add(FVector(A.X, A.Y, H));
        OutNormals.Add(Normal);
        OutNormals.Add(Normal);
        OutNormals.Add(Normal);
        OutNormals.Add(Normal);

        OutTris.Add(I0 + 0); OutTris.Add(I0 + 1); OutTris.Add(I0 + 2);
        OutTris.Add(I0 + 0); OutTris.Add(I0 + 2); OutTris.Add(I0 + 3);
    }

    // Roof: a fan from the footprint centroid, facing up.
    FVector2D Centroid(0.0f, 0.0f);
    for (const FVector2D& P : Points)
    {
        Centroid += P;
    }
    Centroid /= static_cast<float>(Count);

    const int32 CenterIndex = OutVerts.Num();
    OutVerts.Add(FVector(Centroid.X, Centroid.Y, H));
    OutNormals.Add(FVector(0.0f, 0.0f, 1.0f));

    for (int32 i = 0; i < Count; ++i)
    {
        const FVector2D& A = Points[i];
        const FVector2D& B = Points[(i + 1) % Count];

        const int32 IA = OutVerts.Num();
        OutVerts.Add(FVector(A.X, A.Y, H));
        OutVerts.Add(FVector(B.X, B.Y, H));
        OutNormals.Add(FVector(0.0f, 0.0f, 1.0f));
        OutNormals.Add(FVector(0.0f, 0.0f, 1.0f));

        OutTris.Add(CenterIndex); OutTris.Add(IA); OutTris.Add(IA + 1);
    }

    return (Count * 2) + Count;
}

int32 AProceduralStreet::BuildRoadGeometry(
    const TArray<FVector2D>& CenterlineLocalCm,
    float WidthCm,
    float Zcm,
    TArray<FVector>& OutVerts,
    TArray<int32>& OutTris,
    TArray<FVector>& OutNormals)
{
    if (CenterlineLocalCm.Num() < 2 || WidthCm <= 0.0f)
    {
        return 0;
    }

    const float HalfWidth = WidthCm * 0.5f;
    int32 TrianglesAdded = 0;

    for (int32 i = 0; i + 1 < CenterlineLocalCm.Num(); ++i)
    {
        const FVector2D A = CenterlineLocalCm[i];
        const FVector2D B = CenterlineLocalCm[i + 1];

        FVector2D Dir = B - A;
        if (!Dir.Normalize())
        {
            continue;
        }
        const FVector2D Perp(-Dir.Y, Dir.X);

        const int32 I0 = OutVerts.Num();
        const FVector2D Corners[4] = {
            A + Perp * HalfWidth,
            A - Perp * HalfWidth,
            B - Perp * HalfWidth,
            B + Perp * HalfWidth,
        };
        for (const FVector2D& Corner : Corners)
        {
            OutVerts.Add(FVector(Corner.X, Corner.Y, Zcm));
            OutNormals.Add(FVector(0.0f, 0.0f, 1.0f));
        }

        OutTris.Add(I0 + 0); OutTris.Add(I0 + 1); OutTris.Add(I0 + 2);
        OutTris.Add(I0 + 0); OutTris.Add(I0 + 2); OutTris.Add(I0 + 3);
        TrianglesAdded += 2;
    }

    return TrianglesAdded;
}

void AProceduralStreet::ClearStreet()
{
    if (BuildingsMesh)
    {
        BuildingsMesh->ClearAllMeshSections();
    }
    if (RoadsMesh)
    {
        RoadsMesh->ClearAllMeshSections();
    }
    BuildingCount = 0;
    RoadCount = 0;
    TriangleCount = 0;
}

void AProceduralStreet::BuildStreet(const TArray<FGeoBuilding>& Buildings, const TArray<FGeoRoad>& Roads)
{
    using namespace ProceduralStreetPrivate;

    ClearStreet();

    if (!BuildingMaterial)
    {
        BuildingMaterial = LoadObject<UMaterialInterface>(
            nullptr, TEXT("/Game/Materials/M_StreetBuilding"));
        if (!BuildingMaterial)
        {
            BuildingMaterial = LoadObject<UMaterialInterface>(
                nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
        }
    }
    if (!RoadMaterial)
    {
        RoadMaterial = LoadObject<UMaterialInterface>(
            nullptr, TEXT("/Game/Materials/M_StreetRoad"));
        if (!RoadMaterial)
        {
            RoadMaterial = BuildingMaterial;
        }
    }

    // One section per building keeps per-building collision hulls and picking.
    int32 SectionIndex = 0;
    for (const FGeoBuilding& Building : Buildings)
    {
        TArray<FVector> Verts;
        TArray<int32> Tris;
        TArray<FVector> Normals;
        const int32 Added = BuildBuildingGeometry(
            Building.Footprint, Building.Height, Verts, Tris, Normals);

        if (Added <= 0 || Verts.Num() == 0)
        {
            continue;
        }

        TArray<FVector2D> UVs;
        UVs.Reserve(Verts.Num());
        for (const FVector& V : Verts)
        {
            UVs.Add(PlanarUV(V));
        }

        BuildingsMesh->CreateMeshSection_LinearColor(
            SectionIndex, Verts, Tris, Normals, UVs, BuildingColors, NoTangents, bCreateCollision);
        if (BuildingMaterial)
        {
            BuildingsMesh->SetMaterial(SectionIndex, BuildingMaterial);
        }

        TriangleCount += Tris.Num() / 3;
        ++BuildingCount;
        ++SectionIndex;
    }

    // All roads share one non-colliding section.
    {
        TArray<FVector> Verts;
        TArray<int32> Tris;
        TArray<FVector> Normals;
        for (const FGeoRoad& Road : Roads)
        {
            if (BuildRoadGeometry(Road.Centerline, Road.Width, RoadHeightCm, Verts, Tris, Normals) > 0)
            {
                ++RoadCount;
            }
        }

        if (Verts.Num() > 0)
        {
            TArray<FVector2D> UVs;
            UVs.Reserve(Verts.Num());
            for (const FVector& V : Verts)
            {
                UVs.Add(PlanarUV(V));
            }

            RoadsMesh->CreateMeshSection_LinearColor(
                0, Verts, Tris, Normals, UVs, BuildingColors, NoTangents, false);
            if (RoadMaterial)
            {
                RoadsMesh->SetMaterial(0, RoadMaterial);
            }
            TriangleCount += Tris.Num() / 3;
        }
    }

    UE_LOG(LogTemp, Log,
        TEXT("[Honeycomb] Street built: %d buildings, %d roads, %d triangles"),
        BuildingCount, RoadCount, TriangleCount);
}
