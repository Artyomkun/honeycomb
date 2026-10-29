#pragma once

#include "HoneycombCore.h"
#include "GameFramework/Actor.h"
#include "HoneycombAvatar.generated.h"

class UPoseableMeshComponent;
class USkeletalMesh;
class UAvatarAutoFitComponent;

/**
 * Owns the player avatar mesh and the component that fits it to the player.
 *
 * The mesh lives on a UPoseableMeshComponent because the fit is applied as bone
 * transforms: UAvatarAutoFitComponent writes a per-bone stretch to the arm chain
 * and a uniform component scale for stature. A plain USkeletalMeshComponent only
 * accepts those transforms through an animation instance, which would move the
 * whole fit logic into an AnimBP for no benefit while the avatar is static.
 * When arm IK or body animation is added later, move the fit into an AnimInstance
 * proxy and swap this component for a USkeletalMeshComponent.
 */
UCLASS(Blueprintable, BlueprintType)
class HONEYCOMB_API AHoneycombAvatar: public AActor
{
    GENERATED_BODY()

    public:
        AHoneycombAvatar();

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Avatar")
        TObjectPtr<UPoseableMeshComponent> BodyMesh;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Avatar")
        TObjectPtr<UAvatarAutoFitComponent> AutoFit;

        /**
         * Mesh applied to BodyMesh on construction.
         *
         * The asset lives in this property instead of on the component because
         * UPoseableMeshComponent keeps its mesh in a private deprecated field, so
         * the only public setter is SetSkinnedAssetAndUpdate. Keeping the asset
         * here means a Blueprint child stores it as an ordinary editable value
         * and previews it, instead of needing a construction script to run once.
         */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avatar")
        TObjectPtr<USkeletalMesh> BodyMeshAsset = nullptr;

        UFUNCTION(BlueprintCallable, Category = "Avatar")
        void SetBodyMeshAsset(USkeletalMesh* Mesh);

        /** Fits without waiting for the interpolation, e.g. after a calibration step. */
        UFUNCTION(BlueprintCallable, Category = "Avatar")
        void RefitNow();

    protected:
        virtual void OnConstruction(const FTransform& Transform) override;
};
