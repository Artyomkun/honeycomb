#include "AvatarAutoFitComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/SkinnedAsset.h"
#include "HeadMountedDisplayTypes.h"
#include "IMotionController.h"
#include "IXRTrackingSystem.h"
#include "MotionControllerComponent.h"
#include "ReferenceSkeleton.h"

namespace
{
    /** Bone scale must land the fingertip this close to the requested span. */
    constexpr double GWristToleranceCm = 0.5;

    /**
     * The reference skeleton stores every bone parent relative, but the span math
     * needs the shoulder offset and the bone direction in component space, so the
     * ref pose is accumulated once per push.
     */
    void BuildComponentSpacePose(const FReferenceSkeleton& Skeleton, TArray<FTransform>& OutPose)
    {
        const TArray<FTransform>& RefPose = Skeleton.GetRefBonePose();

        OutPose.Reset();
        OutPose.Reserve(RefPose.Num());

        for (int32 BoneIndex = 0; BoneIndex < RefPose.Num(); ++BoneIndex)
        {
            const int32 ParentIndex = BoneIndex > 0 ? Skeleton.GetParentIndex(BoneIndex) : INDEX_NONE;
            if (ParentIndex == INDEX_NONE || !OutPose.IsValidIndex(ParentIndex))
            {
                OutPose.Add(RefPose[BoneIndex]);
                continue;
            }

            OutPose.Add(RefPose[BoneIndex] * OutPose[ParentIndex]);
        }
    }
}

UAvatarAutoFitComponent::UAvatarAutoFitComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    ArmChainRootBones = {TEXT("upperarm_l"), TEXT("upperarm_r")};
    ArmWristBones = {TEXT("hand_l"), TEXT("hand_r")};
}

void UAvatarAutoFitComponent::BeginPlay()
{
    Super::BeginPlay();

    ResolveTargetMesh();

    if (bFitOnBeginPlay)
    {
        FitNow();
    }
}

void UAvatarAutoFitComponent::ResolveTargetMesh()
{
    if (TargetMesh == nullptr)
    {
        AActor* Owner = GetOwner();
        if (Owner != nullptr)
        {
            TargetMesh = Owner->FindComponentByClass<USkinnedMeshComponent>();
        }
    }

    if (TargetMesh != nullptr && !bBaseScaleCaptured)
    {
        BaseRelativeScale = TargetMesh->GetRelativeScale3D();
        bBaseScaleCaptured = true;
    }

    if (TargetMesh == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("AvatarAutoFit: SkinnedMeshComponent not found on owner, assign TargetMesh manually."));
    }
}

bool UAvatarAutoFitComponent::MeasurePlayerHeight(float& OutHeightCm)
{
    OutHeightCm = 0.0f;

    if (GEngine == nullptr || !GEngine->XRSystem.IsValid())
    {
        return false;
    }

    if (bForceFloorTrackingOrigin)
    {
        GEngine->XRSystem->SetTrackingOrigin(EHMDTrackingOrigin::LocalFloor);
    }

    FQuat DeviceOrientation = FQuat::Identity;
    FVector DevicePosition = FVector::ZeroVector;
    if (!GEngine->XRSystem->GetCurrentPose(IXRTrackingSystem::HMDDeviceId, DeviceOrientation, DevicePosition))
    {
        return false;
    }

    const float RawHeightCm = DevicePosition.Z;
    if (FMath::IsNaN(RawHeightCm) || RawHeightCm < MinPlayerHeightCm * 0.5f)
    {
        return false;
    }

    OutHeightCm = FMath::Clamp(RawHeightCm, MinPlayerHeightCm, MaxPlayerHeightCm);
    return true;
}

void UAvatarAutoFitComponent::FitNow()
{
    float HeightCm = 0.0f;
    if (MeasurePlayerHeight(HeightCm))
    {
        UpdateTargetScale(HeightCm);
    }
    else if (bUseDefaultHeightWithoutXR)
    {
        UpdateTargetScale(DefaultPlayerHeightCm);
    }

    TimeSinceLastRefit = 0.0f;
    TimeWithoutXR = 0.0f;
}

void UAvatarAutoFitComponent::UpdateTargetScale(float NewHeightCm)
{
    if (AuthoredHeightCm <= KINDA_SMALL_NUMBER || NewHeightCm <= 0.0f)
    {
        return;
    }

    MeasuredPlayerHeightCm = NewHeightCm;
    TargetScale = FMath::Clamp(NewHeightCm / AuthoredHeightCm, 0.25f, 4.0f);

    if (!bFitCompleted)
    {
        bFitCompleted = true;
        OnAvatarFitted(MeasuredPlayerHeightCm, TargetScale);
    }
}

void UAvatarAutoFitComponent::ResolveHandControllers()
{
    if (LeftHandController != nullptr && RightHandController != nullptr)
    {
        return;
    }

    AActor* Owner = GetOwner();
    if (Owner == nullptr)
    {
        return;
    }

    TArray<UMotionControllerComponent*> Hands;
    Owner->GetComponents(Hands);

    for (UMotionControllerComponent* Hand : Hands)
    {
        if (Hand == nullptr)
        {
            continue;
        }

        const FName Source = Hand->MotionSource;
        if (Source == IMotionController::LeftHandSourceId && LeftHandController == nullptr)
        {
            LeftHandController = Hand;
        }
        else if (Source == IMotionController::RightHandSourceId && RightHandController == nullptr)
        {
            RightHandController = Hand;
        }
    }
}

bool UAvatarAutoFitComponent::MeasureArmSpan(float& OutArmSpanCm)
{
    OutArmSpanCm = 0.0f;

    ResolveHandControllers();
    if (LeftHandController == nullptr || RightHandController == nullptr)
    {
        return false;
    }

    if (!LeftHandController->IsTracked() || !RightHandController->IsTracked())
    {
        return false;
    }

    const FVector LeftPos = LeftHandController->GetComponentLocation();
    const FVector RightPos = RightHandController->GetComponentLocation();

    const float SpanCm = FVector::Dist(LeftPos, RightPos);
    if (SpanCm < KINDA_SMALL_NUMBER)
    {
        return false;
    }

    if (SpanCm < MinArmSpanCm * 0.5f || SpanCm > MaxArmSpanCm * 1.2f)
    {
        return false;
    }

    OutArmSpanCm = FMath::Clamp(SpanCm, MinArmSpanCm, MaxArmSpanCm);

    if (!bArmSpanCalibrated || OutArmSpanCm > MeasuredArmSpanCm)
    {
        MeasuredArmSpanCm = OutArmSpanCm;
    }

    return true;
}

void UAvatarAutoFitComponent::ResetArmSpanCalibration()
{
    bArmSpanCalibrated = false;
    MeasuredArmSpanCm = 0.0f;
    TimeSinceLastArmSample = 0.0f;
}

void UAvatarAutoFitComponent::SetMeasuredArmSpan(float ArmSpanCm)
{
    if (ArmSpanCm <= KINDA_SMALL_NUMBER)
    {
        return;
    }

    MeasuredArmSpanCm = FMath::Clamp(ArmSpanCm, MinArmSpanCm, MaxArmSpanCm);
    bArmSpanCalibrated = true;
    bAutoFitArmSpan = false;

    PushArmBoneFit();
}

void UAvatarAutoFitComponent::ApplyFitImmediate()
{
    CurrentFitScale = TargetScale;

    if (TargetMesh != nullptr)
    {
        TargetMesh->SetRelativeScale3D(BaseRelativeScale * CurrentFitScale);
    }

    PushArmBoneFit();
}

float UAvatarAutoFitComponent::SolveArmBoneScale(const TArray<FTransform>& RefComponentSpace, const FReferenceSkeleton& Skeleton, int32 SideIndex) const
{
    if (SideIndex < 0 || SideIndex >= ArmChainRootBones.Num())
    {
        return 1.0f;
    }

    if (MeasuredArmSpanCm <= KINDA_SMALL_NUMBER || AuthoredArmSpanCm <= KINDA_SMALL_NUMBER)
    {
        return 1.0f;
    }

    const int32 RootIndex = Skeleton.FindBoneIndex(ArmChainRootBones[SideIndex]);
    if (RootIndex == INDEX_NONE || !RefComponentSpace.IsValidIndex(RootIndex))
    {
        return 1.0f;
    }

    // The reference skeleton stores bones parent relative, so the shoulder offset
    // has to be read from the accumulated component space pose.

    // X is measured from the mesh origin because the component scale is applied
    // about that origin, and the generated body is symmetric across it.
    const double PivotX = FMath::Abs(RefComponentSpace[RootIndex].GetTranslation().X);

    // Arm span is a T-pose measurement and the generator builds the chain with
    // span / 2 == pivot + chain length, so both the authored and the wanted
    // span use that relation. The pose angle and the hand bone length cancel
    // out, and only the pivot offset has to be read from the skeleton.
    const double StatureScale = FMath::Max(static_cast<double>(CurrentFitScale), static_cast<double>(KINDA_SMALL_NUMBER));
    const double AuthoredChain = 0.5 * AuthoredArmSpanCm - PivotX;
    const double WantedChain = 0.5 * MeasuredArmSpanCm / StatureScale - PivotX;
    if (AuthoredChain < 1.0)
    {
        return 1.0f;
    }

    const double Solved = WantedChain / AuthoredChain;
    return FMath::Clamp(static_cast<float>(Solved), MinArmBoneScale, MaxArmBoneScale);
}

void UAvatarAutoFitComponent::PushArmBoneFit()
{
    UPoseableMeshComponent* PoseMesh = Cast<UPoseableMeshComponent>(TargetMesh);
    if (PoseMesh == nullptr)
    {
        bArmBoneFitApplied = false;
        return;
    }

    USkinnedAsset* Asset = PoseMesh->GetSkinnedAsset();
    if (Asset == nullptr)
    {
        bArmBoneFitApplied = false;
        return;
    }

    const FReferenceSkeleton& Skeleton = Asset->GetRefSkeleton();

    TArray<FTransform> RefComponentSpace;
    BuildComponentSpacePose(Skeleton, RefComponentSpace);

    TargetArmBoneScale = SolveArmBoneScale(RefComponentSpace, Skeleton, 0);

    const bool bStretchChanged = !FMath::IsNearlyEqual(TargetArmBoneScale, CurrentArmBoneScale, 0.0005f);
    if (bStretchChanged)
    {
        CurrentArmBoneScale = TargetArmBoneScale;
    }

    const double Stretch = CurrentArmBoneScale;

    for (int32 SideIndex = 0; SideIndex < ArmChainRootBones.Num() && SideIndex < ArmWristBones.Num(); ++SideIndex)
    {
        const int32 RootIndex = Skeleton.FindBoneIndex(ArmChainRootBones[SideIndex]);
        const int32 WristIndex = Skeleton.FindBoneIndex(ArmWristBones[SideIndex]);
        if (RootIndex == INDEX_NONE || WristIndex == INDEX_NONE || !RefComponentSpace.IsValidIndex(RootIndex) || !RefComponentSpace.IsValidIndex(WristIndex))
        {
            UE_LOG(LogTemp, Warning, TEXT("AvatarAutoFit: arm bone '%s' missing, arm fit skipped."), *ArmChainRootBones[SideIndex].ToString());
            bArmBoneFitApplied = false;
            return;
        }

        const FTransform& RootCs = RefComponentSpace[RootIndex];
        const FVector Direction = (RefComponentSpace[WristIndex].GetTranslation() - RootCs.GetTranslation()).GetSafeNormal();
        if (Direction.IsNearlyZero())
        {
            bArmBoneFitApplied = false;
            return;
        }

        // The ref pose is stored parent relative and, because this mesh is
        // authored in FBX centimetre units, it carries a uniform root scale. The
        // stretch therefore has to reuse the bone's own basis and scale and only
        // push the axis that points down the bone.
        const FQuat RootRotation = RootCs.GetRotation();
        const FVector UnitAxes[3] = {
            RootRotation.RotateVector(FVector::XAxisVector),
            RootRotation.RotateVector(FVector::YAxisVector),
            RootRotation.RotateVector(FVector::ZAxisVector)};

        int32 StretchAxis = 0;
        float BestAlignment = -1.0f;
        for (int32 AxisIndex = 0; AxisIndex < 3; ++AxisIndex)
        {
            const float Alignment = FMath::Abs(FVector::DotProduct(UnitAxes[AxisIndex], Direction));
            if (Alignment > BestAlignment)
            {
                BestAlignment = Alignment;
                StretchAxis = AxisIndex;
            }
        }

        if (bStretchChanged)
        {
            // The bone keeps its own origin, so the shoulder joint stays put, and
            // children inherit the scale, which moves the whole chain together.
            const FVector StretchAxes[3] = {
                UnitAxes[0] * (StretchAxis == 0 ? Stretch : 1.0),
                UnitAxes[1] * (StretchAxis == 1 ? Stretch : 1.0),
                UnitAxes[2] * (StretchAxis == 2 ? Stretch : 1.0)};

            const FMatrix StretchedMatrix(
                StretchAxes[0] * RootCs.GetScale3D().X,
                StretchAxes[1] * RootCs.GetScale3D().Y,
                StretchAxes[2] * RootCs.GetScale3D().Z,
                RootCs.GetTranslation());

            PoseMesh->SetBoneTransformByName(ArmChainRootBones[SideIndex], FTransform(StretchedMatrix), EBoneSpaces::ComponentSpace);
        }

        FVector PredictedLocal = RootRotation.UnrotateVector(RefComponentSpace[WristIndex].GetTranslation() - RootCs.GetTranslation());
        if (StretchAxis == 0)
        {
            PredictedLocal.X *= Stretch;
        }
        else if (StretchAxis == 1)
        {
            PredictedLocal.Y *= Stretch;
        }
        else
        {
            PredictedLocal.Z *= Stretch;
        }

        const FVector Predicted = RootCs.GetTranslation() + RootRotation.RotateVector(PredictedLocal);
        const FVector Actual = PoseMesh->GetBoneLocationByName(ArmWristBones[SideIndex], EBoneSpaces::ComponentSpace);
        if (FVector::Dist(Predicted, Actual) > GWristToleranceCm)
        {
            UE_LOG(LogTemp, Warning, TEXT("AvatarAutoFit: wrist '%s' landed at %s, predicted %s."), *ArmWristBones[SideIndex].ToString(), *Actual.ToCompactString(), *Predicted.ToCompactString());
            bArmBoneFitApplied = false;
            return;
        }
    }

    bArmBoneFitApplied = true;
}

void UAvatarAutoFitComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    ResolveTargetMesh();
    if (TargetMesh == nullptr)
    {
        return;
    }

    if (!bFitCompleted)
    {
        float HeightCm = 0.0f;
        if (MeasurePlayerHeight(HeightCm))
        {
            UpdateTargetScale(HeightCm);
        }
        else
        {
            TimeWithoutXR += DeltaTime;
            if (bUseDefaultHeightWithoutXR && TimeWithoutXR >= XRLookupTimeoutSeconds)
            {
                UpdateTargetScale(DefaultPlayerHeightCm);
            }
        }
    }
    else if (bContinuousRefit)
    {
        TimeSinceLastRefit += DeltaTime;
        if (TimeSinceLastRefit >= RefitIntervalSeconds)
        {
            TimeSinceLastRefit = 0.0f;
            float HeightCm = 0.0f;
            if (MeasurePlayerHeight(HeightCm))
            {
                UpdateTargetScale(HeightCm);
            }
        }
    }

    CurrentFitScale = FMath::FInterpTo(CurrentFitScale, TargetScale, DeltaTime, FitInterpSpeed);
    if (!FMath::IsNearlyEqual(CurrentFitScale, TargetScale, 0.0005f) || !bBaseScaleCaptured)
    {
        TargetMesh->SetRelativeScale3D(BaseRelativeScale * CurrentFitScale);
    }

    if (bAutoFitArmSpan)
    {
        TimeSinceLastArmSample += DeltaTime;
        TimeSinceArmStart += DeltaTime;

        if (TimeSinceLastArmSample >= ArmSpanSampleInterval)
        {
            TimeSinceLastArmSample = 0.0f;

            float SpanCm = 0.0f;
            if (MeasureArmSpan(SpanCm))
            {
                bArmSpanCalibrated = true;
            }
        }

        if (bArmSpanCalibrated && TimeSinceArmStart >= WarmupSeconds)
        {
            bAutoFitArmSpan = false;
        }
    }

    if (bPerBoneArmFit && bArmSpanCalibrated && MeasuredArmSpanCm > KINDA_SMALL_NUMBER)
    {
        PushArmBoneFit();
    }
}
