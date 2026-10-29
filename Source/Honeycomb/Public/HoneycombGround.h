#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HoneycombGround.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class UProceduralMeshComponent;

/**
 * Procedural honeycomb floor: one hex prism per cell, built at runtime.
 *
 * Flat-top axial hex layout centred on the actor origin. Every cell is its own
 * mesh section with its own dynamic material instance, so a cell can be tinted
 * without rebuilding geometry. Cell top heights come from deterministic value
 * noise sampled on the cell centre, which gives a rolling "honeycomb" floor
 * that is identical on every run and needs no baked asset.
 *
 * The pure hex maths (CellToLocal, LocalToCell, CellDistance, CellNeighbors,
 * EnumerateCells, ValueNoise2D) is static and world independent, so tests can
 * exercise it without spawning an actor.
 */
UCLASS(Blueprintable, BlueprintType)
class HONEYCOMB_API AHoneycombGround : public AActor
{
    GENERATED_BODY()

public:
    AHoneycombGround();

    /** Regenerates every cell. Call after changing any grid property. */
    UFUNCTION(BlueprintCallable, Category = "Honeycomb|Ground")
    void RebuildGrid();

    UFUNCTION(BlueprintPure, Category = "Honeycomb|Ground")
    int32 GetCellCount() const { return Cells.Num(); }

    /** Vertices in the whole grid, valid after RebuildGrid. */
    UFUNCTION(BlueprintPure, Category = "Honeycomb|Ground")
    int32 GetGridVertexCount() const { return GridVertexCount; }

    /** Triangles in the whole grid, valid after RebuildGrid. */
    UFUNCTION(BlueprintPure, Category = "Honeycomb|Ground")
    int32 GetGridTriangleCount() const { return GridTriangleCount; }

    /** Cell under a world position, honouring the actor transform. */
    UFUNCTION(BlueprintPure, Category = "Honeycomb|Ground")
    bool WorldToCell(const FVector& WorldLocation, FIntPoint& OutCell) const;

    /** World-space centre of a cell top face. */
    UFUNCTION(BlueprintPure, Category = "Honeycomb|Ground")
    FVector GetCellCenter(const FIntPoint& Cell) const;

    /** Top height of a cell relative to the actor origin, including relief. */
    UFUNCTION(BlueprintPure, Category = "Honeycomb|Ground")
    float GetCellHeight(const FIntPoint& Cell) const;

    UFUNCTION(BlueprintPure, Category = "Honeycomb|Ground")
    bool ContainsCell(const FIntPoint& Cell) const;

    /** Tints one cell with HighlightColor. Pass a cell outside the grid to clear. */
    UFUNCTION(BlueprintCallable, Category = "Honeycomb|Ground")
    void SetHighlightedCell(const FIntPoint& Cell);

    UFUNCTION(BlueprintCallable, Category = "Honeycomb|Ground")
    void ClearHighlight();

    UFUNCTION(BlueprintPure, Category = "Honeycomb|Ground")
    bool GetHighlightedCell(FIntPoint& OutCell) const;

    UFUNCTION(BlueprintPure, Category = "Honeycomb|Ground")
    FLinearColor GetCellColor(const FIntPoint& Cell) const;

    UFUNCTION(BlueprintPure, Category = "Honeycomb|Ground")
    UProceduralMeshComponent* GetGroundMesh() const { return GroundMesh; }

    // --- flat-top axial hex maths (static, no world needed) ---------------

    /** Cell centre in actor-local space. Radius is the centre-to-corner distance. */
    UFUNCTION(BlueprintPure, Category = "Honeycomb|Ground|Hex")
    static FVector CellToLocal(const FIntPoint& Cell, float Radius);

    /** Inverse of CellToLocal. False when the point is outside the grid. */
    UFUNCTION(BlueprintPure, Category = "Honeycomb|Ground|Hex")
    static bool LocalToCell(const FVector& LocalLocation, float Radius, int32 Rings, FIntPoint& OutCell);

    /** Hex grid distance: number of steps between two cells. */
    UFUNCTION(BlueprintPure, Category = "Honeycomb|Ground|Hex")
    static int32 CellDistance(const FIntPoint& A, const FIntPoint& B);

    /** The six cells sharing an edge with Cell. */
    UFUNCTION(BlueprintPure, Category = "Honeycomb|Ground|Hex")
    static void CellNeighbors(const FIntPoint& Cell, TArray<FIntPoint>& OutNeighbors);

    /** Every cell within Rings steps of the origin cell. Rings 0 is the origin only. */
    UFUNCTION(BlueprintPure, Category = "Honeycomb|Ground|Hex")
    static void EnumerateCells(int32 Rings, TArray<FIntPoint>& OutCells);

    /** Number of cells within Rings steps of the origin: 1 + 3 * Rings * (Rings + 1). */
    UFUNCTION(BlueprintPure, Category = "Honeycomb|Ground|Hex")
    static int32 ComputeCellCountForRings(int32 Rings);

    /** Deterministic value noise in [0, 1]; same input always gives the same output. */
    UFUNCTION(BlueprintPure, Category = "Honeycomb|Ground|Hex")
    static float ValueNoise2D(float X, float Y);

protected:
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void BeginPlay() override;

    /** Radius after the inter-cell gap is subtracted, never degenerate. */
    float GetEffectiveRadius() const;

    float ComputeReliefHeight(const FVector2D& CenterLocal) const;

    /** Appends one closed hex prism. Bottom sits CellDepth below the top face. */
    static void BuildCellGeometry(
        const FVector2D& CenterLocal,
        float Radius,
        float TopHeight,
        float Depth,
        TArray<FVector>& OutVerts,
        TArray<int32>& OutTris,
        TArray<FVector>& OutNormals,
        TArray<FVector2D>& OutUVs,
        TArray<FLinearColor>& OutColors);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Honeycomb|Ground")
    UProceduralMeshComponent* GroundMesh;

    /** Cells from the origin out to this many steps. 0 = single cell, 3 = 37 cells. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Honeycomb|Ground", meta = (ClampMin = "0", UIMin = "0", UIMax = "8"))
    int32 Rings = 3;

    /** Centre-to-corner distance of a cell, in cm. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Honeycomb|Ground", meta = (ClampMin = "1.0", UIMin = "50.0", UIMax = "400.0"))
    float CellRadius = 100.0f;

    /** Empty space between neighbouring cells, in cm. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Honeycomb|Ground", meta = (ClampMin = "0.0", UIMax = "60.0"))
    float CellGap = 12.0f;

    /** Prism depth below the top face, in cm. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Honeycomb|Ground", meta = (ClampMin = "1.0", UIMin = "5.0", UIMax = "200.0"))
    float CellDepth = 30.0f;

    /** Peak cell-to-cell height variation, in cm. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Honeycomb|Ground", meta = (ClampMin = "0.0", UIMax = "60.0"))
    float ReliefScale = 14.0f;

    /** Value-noise frequency per cm. Lower is smoother. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Honeycomb|Ground", meta = (ClampMin = "0.0", UIMax = "0.2"))
    float ReliefFrequency = 0.012f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Honeycomb|Ground|Look")
    FLinearColor CellColor = FLinearColor(0.78f, 0.55f, 0.16f, 1.0f);

    /** Used for every other cell so the comb pattern reads at a glance. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Honeycomb|Ground|Look")
    FLinearColor CellColorAlt = FLinearColor(0.55f, 0.36f, 0.08f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Honeycomb|Ground|Look")
    FLinearColor HighlightColor = FLinearColor(1.0f, 0.93f, 0.55f, 1.0f);

    /** Falls back to the engine basic shape material when unset. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Honeycomb|Ground|Look")
    UMaterialInterface* CellMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Honeycomb|Ground|Look")
    FName ColorParameterName = TEXT("Color");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Honeycomb|Ground|Collision")
    bool bCreateCollision = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Honeycomb|Ground|Collision")
    bool bGenerateOverlapEvents = false;

private:
    void ApplyCellColor(int32 CellIndex, const FLinearColor& Color);

    /** Cell order from EnumerateCells, so section index == cell index. */
    TArray<FIntPoint> Cells;

    /** One dynamic material per section; same order as Cells. */
    UPROPERTY(Transient)
    TArray<TObjectPtr<UMaterialInstanceDynamic>> CellMaterials;

    FIntPoint HighlightedCell = FIntPoint(0, 0);
    bool bHasHighlight = false;
    int32 GridVertexCount = 0;
    int32 GridTriangleCount = 0;
};
