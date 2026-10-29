#include "HoneycombAvatar.h"
#include "AvatarAutoFitComponent.h"
#include "Components/PoseableMeshComponent.h"

AHoneycombAvatar::AHoneycombAvatar()
{
    PrimaryActorTick.bCanEverTick = false;

    BodyMesh = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("BodyMesh"));
    SetRootComponent(BodyMesh);

    AutoFit = CreateDefaultSubobject<UAvatarAutoFitComponent>(TEXT("AutoFit"));
    AutoFit->TargetMesh = BodyMesh;
}

void AHoneycombAvatar::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    if (BodyMeshAsset != nullptr && BodyMesh != nullptr && BodyMesh->GetSkinnedAsset() != BodyMeshAsset)
    {
        SetBodyMeshAsset(BodyMeshAsset);
    }
}

void AHoneycombAvatar::SetBodyMeshAsset(USkeletalMesh* Mesh)
{
    if (BodyMesh == nullptr || BodyMesh->GetSkinnedAsset() == Mesh)
    {
        return;
    }

    BodyMesh->SetSkinnedAssetAndUpdate(Mesh);
}

void AHoneycombAvatar::RefitNow()
{
    if (AutoFit != nullptr)
    {
        AutoFit->ApplyFitImmediate();
    }
}
