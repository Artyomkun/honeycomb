#include "MoonScene.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/Engine.h"
#include "GameManager.h"
#include "Blueprint/UserWidget.h"

AMoonScene::AMoonScene()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AMoonScene::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearTimer(CuratorIntroTimerHandle);
    GetWorldTimerManager().ClearTimer(MeditationTimerHandle);
    GetWorldTimerManager().ClearTimer(RealityCreationTimerHandle);
    GetWorldTimerManager().ClearTimer(CosmicUnityTimerHandle);
    GetWorldTimerManager().ClearTimer(CelebrationTimerHandle);
    GetWorldTimerManager().ClearTimer(PortalAnimationTimerHandle);

    Super::EndPlay(EndPlayReason);
}

void AMoonScene::BeginPlay()
{
    Super::BeginPlay();

    GameManager = Cast<AGameManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameManager::StaticClass()));

    if (GameManager)
    {
        if (GameManager->GetCurrentChakraLevel() < 12 || GameManager->GetCurrentDimension() < 5)
        {
            UE_LOG(LogTemp, Warning, TEXT("Player not ready for Moon. Returning to main world."));
            UGameplayStatics::OpenLevel(GetWorld(), FName("MainWorld"));
            return;
        }
    }

    InitializeMoonScene();
    StartMoonSequence();
}

void AMoonScene::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsChallengeActive)
    {
        UpdateChallenge(DeltaTime);
    }

    if (Earth)
    {
        Earth->AddActorWorldRotation(FRotator(0.0f, DeltaTime * 5.0f, 0.0f));
    }

    if (Stars)
    {
        Stars->AddActorWorldRotation(FRotator(0.0f, DeltaTime * 10.0f, 0.0f));
    }
}

void AMoonScene::InitializeMoonScene()
{
    SetupMoonEnvironment();
    CreateCurator();
    SetupFinalChallenge();
}

void AMoonScene::SetupMoonEnvironment()
{
    if (MoonSurface)
    {
        MoonSurface->SetActorScale3D(FVector(1000.0f));

        for (int32 i = 0; i < 50; i++)
        {
            FVector CraterPos = FMath::VRand() * 500.0f;
            CraterPos.Z = 0.0f;
            CreateCrater(CraterPos, FMath::RandRange(5.0f, 20.0f));
        }
    }

    if (Earth)
    {
        Earth->SetActorLocation(FVector(0.0f, 0.0f, 1000.0f));
        Earth->SetActorScale3D(FVector(100.0f));
    }

    if (Portal)
    {
        Portal->SetActorLocation(FVector(200.0f, 0.0f, 0.0f));
        Portal->SetActorHiddenInGame(true);
    }

    if (AmbientAudio)
    {
        AmbientAudio->Play();
    }
}

void AMoonScene::CreateCrater(const FVector& Position, float Radius)
{
    UWorld* World = GetWorld();
    if (!World) return;

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;

    AActor* Crater = World->SpawnActor<AActor>(AActor::StaticClass(), Position, FRotator::ZeroRotator, SpawnParams);
    if (Crater)
    {
        UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(Crater);
        MeshComp->RegisterComponent();
        Crater->SetRootComponent(MeshComp);

        UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere"));
        if (SphereMesh)
        {
            MeshComp->SetStaticMesh(SphereMesh);
            MeshComp->SetWorldScale3D(FVector(Radius, Radius, Radius * 0.3f));
        }

        UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(
            LoadObject<UMaterial>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial")), this);
        if (Mat)
        {
            Mat->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.3f, 0.3f, 0.3f));
            MeshComp->SetMaterial(0, Mat);
        }
    }
}

void AMoonScene::CreateCurator()
{
    if (!Curator)
    {
        UWorld* World = GetWorld();
        if (!World) return;

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;

        Curator = World->SpawnActor<AActor>(AActor::StaticClass(), CuratorPosition.GetLocation(), FRotator::ZeroRotator, SpawnParams);
        if (Curator)
        {
            UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(Curator);
            MeshComp->RegisterComponent();
            Curator->SetRootComponent(MeshComp);

            UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere"));
            if (SphereMesh)
            {
                MeshComp->SetStaticMesh(SphereMesh);
                MeshComp->SetWorldScale3D(FVector(3.0f));
            }

            if (CuratorMaterial)
            {
                MeshComp->SetMaterial(0, CuratorMaterial);
            }
            else
            {
                UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(
                    LoadObject<UMaterial>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial")), this);
                if (Mat)
                {
                    Mat->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.0f, 1.0f, 1.0f));
                    Mat->SetScalarParameterValue(TEXT("Metallic"), 1.0f);
                    Mat->SetScalarParameterValue(TEXT("Roughness"), 0.2f);
                    MeshComp->SetMaterial(0, Mat);
                }
            }

            UPointLightComponent* Light = NewObject<UPointLightComponent>(Curator);
            Light->RegisterComponent();
            Light->AttachToComponent(Curator->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
            Light->SetLightColor(FLinearColor(0.0f, 1.0f, 1.0f));
            Light->SetIntensity(2000.0f);
            Light->SetAttenuationRadius(1000.0f);
        }
    }
}

void AMoonScene::SetupFinalChallenge()
{
    if (ChallengeOrb)
    {
        ChallengeOrb->SetActorHiddenInGame(true);
    }

    if (ChallengePositions.Num() == 0)
    {
        for (int32 i = 0; i < 3; i++)
        {
            FVector Location = FVector(100.0f + i * 50.0f, 0.0f, 0.0f);
            AActor* Pos = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), Location, FRotator::ZeroRotator);
            if (Pos)
            {
                ChallengePositions.Add(Pos);
            }
        }
    }
}

void AMoonScene::StartMoonSequence()
{
    GetWorldTimerManager().SetTimer(MeditationTimerHandle, this, &AMoonScene::CuratorGreeting, 2.0f, false);
}

void AMoonScene::CuratorGreeting()
{
    UE_LOG(LogTemp, Log, TEXT("Curator: Welcome to the Moon, seeker of truth."));

    if (Curator)
    {
        Curator->SetActorScale3D(FVector::ZeroVector);
        CuratorIntroScaleTime = 0.0f;
        GetWorldTimerManager().SetTimer(CuratorIntroTimerHandle, this, &AMoonScene::AnimateCuratorIntro, 0.02f, true);
    }

    GetWorldTimerManager().SetTimer(MeditationTimerHandle, [this]()
    {
        UE_LOG(LogTemp, Log, TEXT("Curator: You have traveled through 54 worlds and 12 chakra levels."));
        GetWorldTimerManager().SetTimer(MeditationTimerHandle, [this]()
        {
            UE_LOG(LogTemp, Log, TEXT("Curator: To become the Curator, you must pass the final trials."));
            GetWorldTimerManager().SetTimer(MeditationTimerHandle, [this]()
            {
                UE_LOG(LogTemp, Log, TEXT("Curator: Are you ready for the final challenge?"));
                bIsCuratorMet = true;
                StartFinalChallenges();
            }, 2.0f, false);
        }, 2.0f, false);
    }, 3.0f, false);
}

void AMoonScene::AnimateCuratorIntro()
{
    if (!Curator)
    {
        GetWorldTimerManager().ClearTimer(CuratorIntroTimerHandle);
        return;
    }

    float Delta = FMath::Max(GetWorld()->GetDeltaSeconds(), 1.0f / 60.0f);
    CuratorIntroScaleTime += Delta;
    float Scale = FMath::Lerp(0.0f, 3.0f, FMath::Clamp(CuratorIntroScaleTime / 2.0f, 0.0f, 1.0f));
    Curator->SetActorScale3D(FVector(Scale));

    if (CuratorIntroScaleTime >= 2.0f)
    {
        Curator->SetActorScale3D(FVector(3.0f));
        GetWorldTimerManager().ClearTimer(CuratorIntroTimerHandle);
    }
}

void AMoonScene::StartFinalChallenges()
{
    UE_LOG(LogTemp, Log, TEXT("Starting final challenges..."));

    if (ChallengeUI)
    {
        ChallengeUI->SetVisibility(ESlateVisibility::Visible);
    }

    MeditationChallenge();
}

void AMoonScene::MeditationChallenge()
{
    UE_LOG(LogTemp, Log, TEXT("Challenge 1: Meditation in cosmic silence"));

    if (ChallengeOrb && ChallengePositions.Num() > 0)
    {
        ChallengeOrb->SetActorLocation(ChallengePositions[0]->GetActorLocation());
        ChallengeOrb->SetActorHiddenInGame(false);
    }

    if (DialogueText)
    {
        DialogueText->SetText(FText::FromString(TEXT("Curator: Approach the orb and stay near it to gather cosmic silence.")));
    }

    MeditationTime = 0.0f;
    CreationHoldTime = 0.0f;
    UnityProgress = 0.0f;
    bIsChallengeActive = true;
}

void AMoonScene::UpdateChallenge(float DeltaTime)
{
    if (!bIsChallengeActive) return;

    if (!IsPlayerNearOrb()) return;

    switch (CurrentChallenge)
    {
        case 0:
            MeditationTime = FMath::Min(MeditationTime + DeltaTime, RequiredMeditationTime);
            AnimateChallengeOrb(ChallengeOrb, DeltaTime);
            if (MeditationTime >= RequiredMeditationTime)
            {
                OnMeditationComplete();
            }
            break;

        case 1:
            if (ChallengePositions.Num() > 1)
            {
                ChallengeOrb->SetActorLocation(ChallengePositions[1]->GetActorLocation());
            }
            CreationHoldTime += DeltaTime;
            if (!bObjectCreated && CreationHoldTime >= RequiredCreationHoldTime)
            {
                bObjectCreated = true;
                CreateRealityObject();
                OnRealityCreationComplete();
            }
            break;

        case 2:
            UnityProgress = FMath::Min(UnityProgress + DeltaTime * 5.0f, RequiredUnity);
            if (ChallengePositions.Num() > 2)
            {
                ChallengeOrb->SetActorLocation(ChallengePositions[2]->GetActorLocation());
            }
            if (UnityProgress >= RequiredUnity)
            {
                OnCosmicUnityComplete();
            }
            break;
    }
}

bool AMoonScene::IsPlayerNearOrb() const
{
    if (!ChallengeOrb) return false;

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    APawn* Pawn = PC ? PC->GetPawn() : nullptr;
    if (!Pawn) return false;

    return FVector::DistSquared(Pawn->GetActorLocation(), ChallengeOrb->GetActorLocation())
        <= FMath::Square(ChallengeProximityRadius);
}

void AMoonScene::OnMeditationComplete()
{
    UE_LOG(LogTemp, Log, TEXT("Meditation complete!"));
    bIsChallengeActive = false;
    CurrentChallenge = 1;

    GetWorldTimerManager().SetTimer(RealityCreationTimerHandle, this, &AMoonScene::RealityCreationChallenge, 1.0f, false);
}

void AMoonScene::RealityCreationChallenge()
{
    UE_LOG(LogTemp, Log, TEXT("Challenge 2: Reality Creation"));

    if (ChallengeOrb && ChallengePositions.Num() > 1)
    {
        ChallengeOrb->SetActorLocation(ChallengePositions[1]->GetActorLocation());
        ChallengeOrb->SetActorHiddenInGame(false);
    }

    if (DialogueText)
    {
        DialogueText->SetText(FText::FromString(TEXT("Curator: Remain close to the orb to channel a new reality.")));
    }

    bObjectCreated = false;
    CreationHoldTime = 0.0f;
    bIsChallengeActive = true;
}

void AMoonScene::OnRealityCreationComplete()
{
    UE_LOG(LogTemp, Log, TEXT("Reality object created!"));
    bIsChallengeActive = false;
    CurrentChallenge = 2;

    GetWorldTimerManager().SetTimer(CosmicUnityTimerHandle, this, &AMoonScene::CosmicUnityChallenge, 1.0f, false);
}

void AMoonScene::CosmicUnityChallenge()
{
    UE_LOG(LogTemp, Log, TEXT("Challenge 3: Cosmic Unity"));

    if (ChallengeOrb && ChallengePositions.Num() > 2)
    {
        ChallengeOrb->SetActorLocation(ChallengePositions[2]->GetActorLocation());
        ChallengeOrb->SetActorHiddenInGame(false);
    }

    if (DialogueText)
    {
        DialogueText->SetText(FText::FromString(TEXT("Curator: Stay with the orb and unite the cosmos around you.")));
    }

    UnityProgress = 0.0f;
    bIsChallengeActive = true;
}

void AMoonScene::OnCosmicUnityComplete()
{
    UE_LOG(LogTemp, Log, TEXT("All challenges completed!"));
    bIsChallengeActive = false;
    Celebration();
}

void AMoonScene::CreateRealityObject()
{
    UWorld* World = GetWorld();
    if (!World) return;

    FVector SpawnLocation = GetActorLocation() + FVector(0.0f, 0.0f, 500.0f);
    RealityObject = World->SpawnActor<AActor>(AActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator);
    if (RealityObject)
    {
        UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(RealityObject);
        MeshComp->RegisterComponent();
        RealityObject->SetRootComponent(MeshComp);

        UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere"));
        if (SphereMesh)
        {
            MeshComp->SetStaticMesh(SphereMesh);
            MeshComp->SetWorldScale3D(FVector(2.0f));
        }

        UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(LoadObject<UMaterial>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial")), this);
        if (Mat)
        {
            Mat->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.0f, 0.0f, 1.0f));
            Mat->SetScalarParameterValue(TEXT("Metallic"), 1.0f);
            Mat->SetScalarParameterValue(TEXT("Roughness"), 0.0f);
            MeshComp->SetMaterial(0, Mat);
        }

        UPointLightComponent* Light = NewObject<UPointLightComponent>(RealityObject);
        Light->RegisterComponent();
        Light->AttachToComponent(RealityObject->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
        Light->SetLightColor(FLinearColor(1.0f, 0.0f, 1.0f));
        Light->SetIntensity(3000.0f);
        Light->SetAttenuationRadius(500.0f);
    }

    bObjectCreated = true;
}

void AMoonScene::Celebration()
{
    UE_LOG(LogTemp, Log, TEXT("Curator: Congratulations! You have passed all trials!"));
    UE_LOG(LogTemp, Log, TEXT("Curator: You are now the new Curator of 54 Lands!"));
    UE_LOG(LogTemp, Log, TEXT("Curator: You can observe the worlds and balance them!"));

    if (Portal)
    {
        Portal->SetActorHiddenInGame(false);
        GetWorldTimerManager().SetTimer(PortalAnimationTimerHandle, [this]()
        {
            if (Portal)
            {
                AnimatePortal(Portal, FMath::Max(GetWorld()->GetDeltaSeconds(), 1.0f / 60.0f));
            }
        }, 0.01f, true);
    }

    if (VictoryUI)
    {
        VictoryUI->SetVisibility(ESlateVisibility::Visible);
    }

    GetWorldTimerManager().SetTimer(CelebrationTimerHandle, [this]()
    {
        GetWorldTimerManager().ClearTimer(PortalAnimationTimerHandle);
        UE_LOG(LogTemp, Log, TEXT("Game completed! You became the Curator!"));
    }, 5.0f, false);
}

void AMoonScene::AnimateChallengeOrb(AActor* Orb, float DeltaTime)
{
    if (!Orb) return;

    float Scale = 1.0f + FMath::Sin(GetWorld()->GetTimeSeconds() * 3.0f) * 0.3f;
    Orb->SetActorScale3D(FVector(Scale));
    Orb->AddActorWorldRotation(FRotator(0.0f, DeltaTime * 60.0f, 0.0f));
}

void AMoonScene::AnimateRealityObject(AActor* Object, float DeltaTime)
{
    if (!Object) return;

    float Scale = 1.0f + FMath::Sin(GetWorld()->GetTimeSeconds() * 5.0f) * 0.2f;
    Object->SetActorScale3D(FVector(Scale) * 2.0f);
    Object->AddActorWorldRotation(FRotator(0.0f, DeltaTime * 90.0f, 0.0f));
}

void AMoonScene::AnimatePortal(AActor* PortalActor, float DeltaTime)
{
    if (!PortalActor) return;

    float Time = GetWorld()->GetTimeSeconds();
    float Scale = FMath::Lerp(0.0f, 1.0f, FMath::Min(Time / 3.0f, 1.0f));
    PortalActor->SetActorScale3D(FVector(Scale));
    PortalActor->AddActorWorldRotation(FRotator(0.0f, DeltaTime * 120.0f, 0.0f));
}

void AMoonScene::ReturnToMainWorld()
{
    if (!GameManager) return;

    if (GameManager->IsPlayerDead())
    {
        UE_LOG(LogTemp, Warning, TEXT("Curator cannot return to Earth. Remains on Moon."));
        return;
    }

    UGameplayStatics::OpenLevel(GetWorld(), FName("MainWorld"));
}

void AMoonScene::CreateNewWorld()
{
    if (DialogueText)
    {
        FString Message = TEXT("Curator: Creating new worlds is the prerogative of higher beings. You can only observe.");
        DialogueText->SetText(FText::FromString(Message));
    }
    UE_LOG(LogTemp, Log, TEXT("Curator: Creating new worlds is not allowed."));
}

void AMoonScene::StopCelebrationEffects()
{
    if (AmbientAudio)
    {
        AmbientAudio->Stop();
    }
    if (CuratorVoice)
    {
        CuratorVoice->Stop();
    }
    UE_LOG(LogTemp, Log, TEXT("AMoonScene: Celebration effects stopped"));
}

void AMoonScene::OnCelebrationComplete()
{
    UE_LOG(LogTemp, Log, TEXT("AMoonScene: Celebration complete"));

    StopCelebrationEffects();

    if (AGameManager* GM = Cast<AGameManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameManager::StaticClass())))
    {
        GM->OnMoonCelebrationComplete();
    }

    if (NextSceneTrigger)
    {
        NextSceneTrigger->Activate();
    }

    OnCelebrationFinished.Broadcast();

    UE_LOG(LogTemp, Log, TEXT("AMoonScene: Post-celebration logic executed"));
}
