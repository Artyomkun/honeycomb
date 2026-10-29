#include "HoneycombGround.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "ProceduralMeshComponent.h"

namespace HoneycombGroundPrivate
{
    /** Flat-top axial neighbour offsets: +X, +X-Y, -Y, -X, -X+Y, +Y. */
    const FIntPoint NeighborOffsets[6] = {
        FIntPoint(1, 0), FIntPoint(1, -1), FIntPoint(0, -1),
        FIntPoint(-1, 0), FIntPoint(-1, 1), FIntPoint(0, 1)
    };

    const float Sqrt3 = 1.7320508f;

    const TArray<FVector2D> EmptyUVs;
    const TArray<FProcMeshTangent> NoTangents;

    uint32 HashCell(int32 X, int32 Y)
    {
        uint32 H = static_cast<uint32>(X) * 374761393u + static_cast<uint32>(Y) * 668265263u;
        H = (H ^ (H >> 13)) * 1274126177u;
        H ^= H >> 16;
        return H;
    }
}

AHoneycombGround::AHoneycombGround()
{
    PrimaryActorTick.bCanEverTick = false;

    GroundMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GroundMesh"));
    SetRootComponent(GroundMesh);

    // One convex hull per cell section: a hex prism is convex, so the player
    // can stand on the floor without a separate blocking volume.
    GroundMesh->bUseAsyncCooking = true;
    GroundMesh->bUseComplexAsSimpleCollision = false;
    GroundMesh->SetCollisionObjectType(ECC_WorldStatic);
    GroundMesh->SetCollisionResponseToAllChannels(ECR_Block);
    GroundMesh->SetGenerateOverlapEvents(false);
    GroundMesh->SetCanEverAffectNavigation(true);
}

// --- lifecycle --------------------------------------------------------------

void AHoneycombGround::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    RebuildGrid();
}

void AHoneycombGround::BeginPlay()
{
    Super::BeginPlay();

    // Actors spawned at runtime from a duplicated archetype can arrive with no
    // sections, so guarantee the grid exists even if OnConstruction was skipped.
    if (GroundMesh && GroundMesh->GetNumSections() == 0)
    {
        RebuildGrid();
    }
}

void AHoneycombGround::RebuildGrid()
{
    using namespace HoneycombGroundPrivate;

    if (!GroundMesh)
    {
        return;
    }

    GroundMesh->ClearAllMeshSections();
    GroundMesh->SetGenerateOverlapEvents(bGenerateOverlapEvents);

    Cells.Reset();
    EnumerateCells(FMath::Max(Rings, 0), Cells);

    UMaterialInterface* BaseMaterial = CellMaterial;
    if (!BaseMaterial)
    {
        BaseMaterial = LoadObject<UMaterialInterface>(
            nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
    }

    const float Radius = GetEffectiveRadius();

    CellMaterials.Reset(Cells.Num());
    GridVertexCount = 0;
    GridTriangleCount = 0;
    bHasHighlight = false;

    TArray<FVector> Verts;
    TArray<int32> Tris;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;
    TArray<FLinearColor> Colors;

    for (int32 Index = 0; Index < Cells.Num(); ++Index)
    {
        const FVector Center = CellToLocal(Cells[Index], Radius);
        const float TopHeight = ComputeReliefHeight(FVector2D(Center.X, Center.Y));

        Verts.Reset();
        Tris.Reset();
        Normals.Reset();
        UVs.Reset();
        Colors.Reset();

        BuildCellGeometry(
            FVector2D(Center.X, Center.Y), Radius, TopHeight, CellDepth,
            Verts, Tris, Normals, UVs, Colors);

        UMaterialInstanceDynamic* Material = nullptr;
        if (BaseMaterial)
        {
            Material = UMaterialInstanceDynamic::Create(BaseMaterial, this);
            Material->SetVectorParameterValue(ColorParameterName, GetCellColor(Cells[Index]));
        }
        CellMaterials.Add(Material);

        GroundMesh->CreateMeshSection_LinearColor(
            Index, Verts, Tris, Normals, UVs, Colors, NoTangents, bCreateCollision);
        GroundMesh->SetMaterial(Index, Material);

        GridVertexCount += Verts.Num();
        GridTriangleCount += Tris.Num() / 3;
    }

    UE_LOG(LogTemp, Verbose, TEXT("[HoneycombGround] %d cells, %d vertices"), Cells.Num(), GridVertexCount);
}

// --- grid queries -----------------------------------------------------------

bool AHoneycombGround::WorldToCell(const FVector& WorldLocation, FIntPoint& OutCell) const
{
    const FVector Local = GetActorTransform().InverseTransformPosition(WorldLocation);
    if (!LocalToCell(Local, GetEffectiveRadius(), Rings, OutCell))
    {
        return false;
    }
    return ContainsCell(OutCell);
}

FVector AHoneycombGround::GetCellCenter(const FIntPoint& Cell) const
{
    const FVector Local = CellToLocal(Cell, GetEffectiveRadius());
    return GetActorTransform().TransformPosition(
        FVector(Local.X, Local.Y, ComputeReliefHeight(FVector2D(Local.X, Local.Y))));
}

float AHoneycombGround::GetCellHeight(const FIntPoint& Cell) const
{
    const FVector Local = CellToLocal(Cell, GetEffectiveRadius());
    return ComputeReliefHeight(FVector2D(Local.X, Local.Y));
}

bool AHoneycombGround::ContainsCell(const FIntPoint& Cell) const
{
    return CellDistance(Cell, FIntPoint(0, 0)) <= FMath::Max(Rings, 0);
}

FLinearColor AHoneycombGround::GetCellColor(const FIntPoint& Cell) const
{
    if (bHasHighlight && Cell == HighlightedCell)
    {
        return HighlightColor;
    }

    // Neighbouring cells in a flat-top comb alternate, which reads as a pattern
    // without needing a texture.
    const int32 Parity = FMath::Abs(Cell.X + Cell.Y) % 2;
    return Parity == 0 ? CellColor : CellColorAlt;
}

void AHoneycombGround::SetHighlightedCell(const FIntPoint& Cell)
{
    if (!ContainsCell(Cell))
    {
        ClearHighlight();
        return;
    }

    if (bHasHighlight && HighlightedCell == Cell)
    {
        return;
    }

    if (bHasHighlight)
    {
        const int32 Previous = Cells.IndexOfByKey(HighlightedCell);
        ApplyCellColor(Previous, GetCellColor(HighlightedCell));
    }

    HighlightedCell = Cell;
    bHasHighlight = true;
    ApplyCellColor(Cells.IndexOfByKey(Cell), HighlightColor);
}

void AHoneycombGround::ClearHighlight()
{
    if (!bHasHighlight)
    {
        return;
    }

    const int32 Previous = Cells.IndexOfByKey(HighlightedCell);
    ApplyCellColor(Previous, GetCellColor(HighlightedCell));
    bHasHighlight = false;
}

bool AHoneycombGround::GetHighlightedCell(FIntPoint& OutCell) const
{
    OutCell = HighlightedCell;
    return bHasHighlight;
}

void AHoneycombGround::ApplyCellColor(int32 CellIndex, const FLinearColor& Color)
{
    if (CellMaterials.IsValidIndex(CellIndex) && CellMaterials[CellIndex])
    {
        CellMaterials[CellIndex]->SetVectorParameterValue(ColorParameterName, Color);
    }
}

// --- geometry ---------------------------------------------------------------

float AHoneycombGround::GetEffectiveRadius() const
{
    // The gap is the empty space between two neighbouring surfaces, so half of
    // it comes off each cell.
    return FMath::Max(CellRadius - CellGap * 0.5f, 1.0f);
}

float AHoneycombGround::ComputeReliefHeight(const FVector2D& CenterLocal) const
{
    if (ReliefScale <= 0.0f || ReliefFrequency <= 0.0f)
    {
        return 0.0f;
    }

    const float Noise = ValueNoise2D(CenterLocal.X * ReliefFrequency, CenterLocal.Y * ReliefFrequency);
    return (Noise - 0.5f) * 2.0f * ReliefScale;
}

void AHoneycombGround::BuildCellGeometry(
    const FVector2D& CenterLocal,
    float Radius,
    float TopHeight,
    float Depth,
    TArray<FVector>& OutVerts,
    TArray<int32>& OutTris,
    TArray<FVector>& OutNormals,
    TArray<FVector2D>& OutUVs,
    TArray<FLinearColor>& OutColors)
{
    const float BottomHeight = TopHeight - FMath::Max(Depth, 0.1f);
    const FLinearColor White(1.0f, 1.0f, 1.0f, 1.0f);

    FVector Corners[6];
    for (int32 Index = 0; Index < 6; ++Index)
    {
        const float Angle = FMath::DegreesToRadians(60.0f * Index);
        Corners[Index] = FVector(
            CenterLocal.X + FMath::Cos(Angle) * Radius,
            CenterLocal.Y + FMath::Sin(Angle) * Radius,
            0.0f);
    }

    // 0 top centre, 1..6 top corners, 7 bottom centre, 8..13 bottom corners,
    // 14..19 top side ring, 20..25 bottom side ring.
    OutVerts.Add(FVector(CenterLocal.X, CenterLocal.Y, TopHeight));
    for (int32 Index = 0; Index < 6; ++Index)
    {
        OutVerts.Add(FVector(Corners[Index].X, Corners[Index].Y, TopHeight));
        OutVerts.Add(FVector(Corners[Index].X, Corners[Index].Y, BottomHeight));
    }
    OutVerts.Add(FVector(CenterLocal.X, CenterLocal.Y, BottomHeight));
    for (int32 Index = 0; Index < 6; ++Index)
    {
        OutVerts.Add(FVector(Corners[Index].X, Corners[Index].Y, TopHeight));
        OutVerts.Add(FVector(Corners[Index].X, Corners[Index].Y, BottomHeight));
    }

    // Caps point straight up/down, sides point radially. The two rings exist
    // only so a corner can carry a cap normal and a side normal at the same time.
    OutNormals.Add(FVector::UpVector);
    for (int32 Index = 0; Index < 6; ++Index)
    {
        OutNormals.Add(FVector::UpVector);
    }
    OutNormals.Add(FVector::DownVector);
    for (int32 Index = 0; Index < 6; ++Index)
    {
        OutNormals.Add(FVector::DownVector);
    }
    for (int32 Index = 0; Index < 6; ++Index)
    {
        const FVector Radial = Corners[Index] - FVector(CenterLocal.X, CenterLocal.Y, 0.0f);
        OutNormals.Add(Radial.GetSafeNormal());
    }
    for (int32 Index = 0; Index < 6; ++Index)
    {
        const FVector Radial = Corners[Index] - FVector(CenterLocal.X, CenterLocal.Y, 0.0f);
        OutNormals.Add(Radial.GetSafeNormal());
    }

    // Top and bottom caps as triangle fans, CCW seen from above.
    for (int32 Index = 0; Index < 6; ++Index)
    {
        const int32 Next = (Index + 1) % 6;
        OutTris.Add(0);
        OutTris.Add(1 + Index);
        OutTris.Add(1 + Next);

        OutTris.Add(7);
        OutTris.Add(8 + Next);
        OutTris.Add(8 + Index);
    }

    // Side quads, wound so the face normal points away from the cell centre.
    for (int32 Index = 0; Index < 6; ++Index)
    {
        const int32 Next = (Index + 1) % 6;
        const int32 TopA = 14 + Index;
        const int32 TopB = 14 + Next;
        const int32 BotA = 20 + Index;
        const int32 BotB = 20 + Next;

        OutTris.Add(TopA);
        OutTris.Add(BotA);
        OutTris.Add(BotB);

        OutTris.Add(TopA);
        OutTris.Add(BotB);
        OutTris.Add(TopB);
    }

    const float InvDiameter = 1.0f / FMath::Max(Radius * 2.0f, 1.0f);
    for (int32 Index = 0; Index < 14; ++Index)
    {
        const FVector& V = OutVerts[Index];
        OutUVs.Add(FVector2D(V.X * InvDiameter + 0.5f, V.Y * InvDiameter + 0.5f));
    }
    for (int32 Index = 0; Index < 6; ++Index)
    {
        const float U = static_cast<float>(Index) / 6.0f;
        OutUVs.Add(FVector2D(U, 0.0f));
        OutUVs.Add(FVector2D(U, 1.0f));
    }

    OutColors.Init(White, OutVerts.Num());
}

// --- hex maths (static, world independent) ----------------------------------

FVector AHoneycombGround::CellToLocal(const FIntPoint& Cell, float Radius)
{
    using namespace HoneycombGroundPrivate;

    const float SafeRadius = FMath::Max(Radius, 0.01f);
    return FVector(
        SafeRadius * 1.5f * Cell.X,
        SafeRadius * Sqrt3 * (Cell.Y + Cell.X * 0.5f),
        0.0f);
}

bool AHoneycombGround::LocalToCell(const FVector& LocalLocation, float Radius, int32 Rings, FIntPoint& OutCell)
{
    using namespace HoneycombGroundPrivate;

    const float SafeRadius = FMath::Max(Radius, 0.01f);
    const float FractionalQ = (2.0f / 3.0f) * LocalLocation.X / SafeRadius;
    const float FractionalR = (-1.0f / 3.0f * LocalLocation.X + Sqrt3 / 3.0f * LocalLocation.Y) / SafeRadius;

    // Cube rounding: pick the nearest cube coordinate, then repair the error
    // that rounding introduced in the third one.
    float CubeX = FractionalQ;
    float CubeZ = FractionalR;
    float CubeY = -CubeX - CubeZ;

    const int32 RoundX = FMath::RoundToInt(CubeX);
    const int32 RoundY = FMath::RoundToInt(CubeY);
    const int32 RoundZ = FMath::RoundToInt(CubeZ);

    const float DiffX = FMath::Abs(CubeX - RoundX);
    const float DiffY = FMath::Abs(CubeY - RoundY);
    const float DiffZ = FMath::Abs(CubeZ - RoundZ);

    int32 ResultX = RoundX;
    int32 ResultZ = RoundZ;
    if (DiffX > DiffY && DiffX > DiffZ)
    {
        ResultX = -RoundY - RoundZ;
    }
    else if (DiffY > DiffZ)
    {
        ResultZ = -RoundX - RoundY;
    }

    OutCell = FIntPoint(ResultX, ResultZ);
    return CellDistance(OutCell, FIntPoint(0, 0)) <= FMath::Max(Rings, 0);
}

int32 AHoneycombGround::CellDistance(const FIntPoint& A, const FIntPoint& B)
{
    const int32 DeltaQ = A.X - B.X;
    const int32 DeltaR = A.Y - B.Y;
    const int32 DeltaS = -DeltaQ - DeltaR;

    return (FMath::Abs(DeltaQ) + FMath::Abs(DeltaR) + FMath::Abs(DeltaS)) / 2;
}

void AHoneycombGround::CellNeighbors(const FIntPoint& Cell, TArray<FIntPoint>& OutNeighbors)
{
    using namespace HoneycombGroundPrivate;

    OutNeighbors.Reset(6);
    for (const FIntPoint& Offset : NeighborOffsets)
    {
        OutNeighbors.Add(FIntPoint(Cell.X + Offset.X, Cell.Y + Offset.Y));
    }
}

void AHoneycombGround::EnumerateCells(int32 Rings, TArray<FIntPoint>& OutCells)
{
    const int32 SafeRings = FMath::Max(Rings, 0);

    OutCells.Reset();
    OutCells.Reserve(ComputeCellCountForRings(SafeRings));

    // One pass over the bounding square with the cube-distance test: looping
    // per ring and re-adding every inner cell would count each cell once per
    // ring it belongs to.
    for (int32 Q = -SafeRings; Q <= SafeRings; ++Q)
    {
        const int32 LowestR = FMath::Max(-SafeRings, -Q - SafeRings);
        const int32 HighestR = FMath::Min(SafeRings, -Q + SafeRings);
        for (int32 R = LowestR; R <= HighestR; ++R)
        {
            if (CellDistance(FIntPoint(Q, R), FIntPoint(0, 0)) <= SafeRings)
            {
                OutCells.Add(FIntPoint(Q, R));
            }
        }
    }
}

int32 AHoneycombGround::ComputeCellCountForRings(int32 Rings)
{
    const int32 SafeRings = FMath::Max(Rings, 0);
    return 1 + 3 * SafeRings * (SafeRings + 1);
}

float AHoneycombGround::ValueNoise2D(float X, float Y)
{
    using namespace HoneycombGroundPrivate;

    const float FloorX = FMath::FloorToFloat(X);
    const float FloorY = FMath::FloorToFloat(Y);
    const float TX = X - FloorX;
    const float TY = Y - FloorY;

    const float SX = TX * TX * (3.0f - 2.0f * TX);
    const float SY = TY * TY * (3.0f - 2.0f * TY);

    const int32 IX = static_cast<int32>(FloorX);
    const int32 IY = static_cast<int32>(FloorY);

    const auto Sample = [](int32 SampleX, int32 SampleY)
    {
        return static_cast<float>(HashCell(SampleX, SampleY) & 0x00FFFFFFu) / static_cast<float>(0x01000000u);
    };

    const float V00 = Sample(IX, IY);
    const float V10 = Sample(IX + 1, IY);
    const float V01 = Sample(IX, IY + 1);
    const float V11 = Sample(IX + 1, IY + 1);

    const float Bottom = FMath::Lerp(V00, V10, SX);
    const float Top = FMath::Lerp(V01, V11, SX);
    return FMath::Lerp(Bottom, Top, SY);
}
