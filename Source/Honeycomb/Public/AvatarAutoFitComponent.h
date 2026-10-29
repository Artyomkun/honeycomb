#pragma once

#include "HoneycombCore.h"
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AvatarAutoFitComponent.generated.h"

class UMotionControllerComponent;
class USkeletalMeshComponent;
class USkinnedMeshComponent;
struct FReferenceSkeleton;

/**
 * Fits the avatar body to the player wearing the headset.
 *
 * Two independent measurements drive two different mechanisms:
 *
 *  - Stature. Applied as a uniform scale on the mesh component. That is exact
 *    for the reference body the generator produces: build_avatar.py derives
 *    every landmark and every girth from stature ratios, so scaling the
 *    reference body by (player height / authored height) yields the same
 *    proportions the generator would have emitted at that height. Stretching
 *    the vertical bone chain instead would be equivalent but would lose
 *    shoulder and hip width, which are proportional to stature but not to
 *    bone length.
 *
 *  - Arm span. Applied per bone on a UPoseableMeshComponent. The arm chain is
 *    sized from the measured span rather than from stature, so a uniform scale
 *    gets a tall player's arms wrong by the difference between their span and
 *    the reference span. Stretching the chain root bone along its own axis
 *    corrects that, and only that bone needs the scale because a parent's scale
 *    is already applied to its children's offsets.
 *
 * Both are read from the tracking system, so nothing here has to be configured
 * per player beyond the authored reference values.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HONEYCOMB_API UAvatarAutoFitComponent: public UActorComponent
{
    GENERATED_BODY()

    public:
        UAvatarAutoFitComponent();

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avatar AutoFit")
        USkinnedMeshComponent* TargetMesh = nullptr;

        /** Stature of the generated reference mesh. Must match the mesh's own build. */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avatar AutoFit")
        float AuthoredHeightCm = 178.0f;

        /** Fingertip-to-fingertip reach of the generated reference mesh. */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avatar AutoFit")
        float AuthoredArmSpanCm = 181.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avatar AutoFit")
        float DefaultPlayerHeightCm = 170.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avatar AutoFit")
        float MinPlayerHeightCm = 90.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avatar AutoFit")
        float MaxPlayerHeightCm = 250.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avatar AutoFit")
        float FitInterpSpeed = 4.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avatar AutoFit")
        bool bFitOnBeginPlay = true;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avatar AutoFit")
        bool bContinuousRefit = true;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avatar AutoFit")
        float RefitIntervalSeconds = 2.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avatar AutoFit")
        bool bForceFloorTrackingOrigin = true;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avatar AutoFit")
        bool bUseDefaultHeightWithoutXR = true;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avatar AutoFit")
        float XRLookupTimeoutSeconds = 3.0f;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Avatar AutoFit")
        float MeasuredPlayerHeightCm = 0.0f;

        /** Stature scale currently applied to the mesh component. */
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Avatar AutoFit")
        float CurrentFitScale = 1.0f;

        // --- Arm span: measured from hand motion controllers ---

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avatar AutoFit|ArmSpan")
        bool bAutoFitArmSpan = true;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avatar AutoFit|ArmSpan")
        UMotionControllerComponent* LeftHandController = nullptr;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avatar AutoFit|ArmSpan")
        UMotionControllerComponent* RightHandController = nullptr;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avatar AutoFit|ArmSpan")
        float MinArmSpanCm = 120.0f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avatar AutoFit|ArmSpan")
        float MaxArmSpanCm = 230.0f;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Avatar AutoFit|ArmSpan")
        float MeasuredArmSpanCm = 0.0f;

        // --- Per-bone arm fitting ---

        /** Off until a span has been measured; the reference mesh is already correct. */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avatar AutoFit|ArmFit")
        bool bPerBoneArmFit = true;

        /** Clamped to the same range metrics.py uses for arm_scale, so a fitted
         *  arm is always a proportion the generator could have produced. */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avatar AutoFit|ArmFit")
        float MinArmBoneScale = 0.70f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avatar AutoFit|ArmFit")
        float MaxArmBoneScale = 1.15f;

        /** Chain root per side. Only these bones are scaled; children inherit. */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avatar AutoFit|ArmFit")
        TArray<FName> ArmChainRootBones;

        /** Wrist bone per side; its parent is used to find the hand direction. */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avatar AutoFit|ArmFit")
        TArray<FName> ArmWristBones;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Avatar AutoFit|ArmFit")
        float CurrentArmBoneScale = 1.0f;

    /** False when the requested arm fit could not be realised (missing bone, clamp hit). */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Avatar AutoFit|ArmFit")
    bool bArmBoneFitApplied = false;

    UFUNCTION(BlueprintCallable, Category = "Avatar AutoFit")
    bool MeasurePlayerHeight(float& OutHeightCm);

    UFUNCTION(BlueprintCallable, Category = "Avatar AutoFit|ArmSpan")
    bool MeasureArmSpan(float& OutArmSpanCm);

    UFUNCTION(BlueprintCallable, Category = "Avatar AutoFit|ArmSpan")
    void ResetArmSpanCalibration();

    /** Injects an already known span, e.g. from a calibration menu, and fits immediately. */
    UFUNCTION(BlueprintCallable, Category = "Avatar AutoFit|ArmSpan")
    void SetMeasuredArmSpan(float ArmSpanCm);

    /** Skips the interpolation, e.g. during calibration or when a test needs a settled fit. */
    UFUNCTION(BlueprintCallable, Category = "Avatar AutoFit")
    void ApplyFitImmediate();

    UFUNCTION(BlueprintCallable, Category = "Avatar AutoFit")
    void FitNow();

    UFUNCTION(BlueprintImplementableEvent, Category = "Avatar AutoFit")
    void OnAvatarFitted(float PlayerHeightCm, float AppliedScale);

    protected:
        virtual void BeginPlay() override;
        virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

        void ResolveTargetMesh();
        void ResolveHandControllers();
        void UpdateTargetScale(float NewHeightCm);

    private:
        /** Stretch of one bone chain along its own axis, from the measured span. */
        float SolveArmBoneScale(const TArray<FTransform>& RefComponentSpace, const FReferenceSkeleton& Skeleton, int32 SideIndex) const;

        void PushArmBoneFit();

        FVector BaseRelativeScale = FVector(1.0f);
        bool bBaseScaleCaptured = false;

        bool bFitCompleted = false;
        float TargetScale = 1.0f;
        float TimeSinceLastRefit = 0.0f;
        float TimeWithoutXR = 0.0f;

        // Arm span: the player must spread their arms; we keep the max observed span.
        bool bArmSpanCalibrated = false;
        float TimeSinceLastArmSample = 0.0f;
        float TimeSinceArmStart = 0.0f;
        float ArmSpanSampleInterval = 0.25f;
        float WarmupSeconds = 6.0f;

        float TargetArmBoneScale = 1.0f;
};
