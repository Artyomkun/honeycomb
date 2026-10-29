#include "HoneycombPlayerPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameManager.h"
#include "HoneycombAvatar.h"
#include "Systems/KriyaYogaSystem.h"
#include "VRUIManager.h"

#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

AHoneycombPlayerPawn::AHoneycombPlayerPawn()
{
    PrimaryActorTick.bCanEverTick = true;

    GetCapsuleComponent()->InitCapsuleSize(34.0f, 88.0f);

    FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
    FirstPersonCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));
    FirstPersonCamera->bUsePawnControlRotation = true;

    bUseControllerRotationYaw = true;
    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    GetCharacterMovement()->JumpZVelocity = 420.0f;
    GetCharacterMovement()->AirControl = 0.25f;

    // The flat view is first person; hide the placeholder skeletal mesh.
    GetMesh()->SetHiddenInGame(true);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AHoneycombPlayerPawn::BeginPlay()
{
    Super::BeginPlay();

    BuildFlatInput();

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = bRunHeld ? RunSpeed : WalkSpeed;
    }

    if (!AvatarClass)
    {
        AvatarClass = LoadClass<AHoneycombAvatar>(nullptr, TEXT("/Game/Avatars/BP_PlayerAvatar.BP_PlayerAvatar_C"));
    }
}

void AHoneycombPlayerPawn::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    UpdateFocus();

    if (bMeditating)
    {
        AGameManager* GM = AGameManager::GetInstance();
        UKriyaYogaSystem* Kriya = GM ? GM->GetKriyaYogaSystem() : nullptr;
        AVRUIManager* UI = AVRUIManager::GetInstance(GetWorld());

        if (Kriya && Kriya->IsKriyaActive())
        {
            if (UI)
            {
                UI->ShowMeditationProgress(Kriya->GetKriyaProgress());
            }
        }
        else
        {
            // Kriya finished (or was cancelled): return to normal HUD.
            bMeditating = false;
            if (UI)
            {
                UI->HideMeditationProgress();
            }
        }
    }
}

void AHoneycombPlayerPawn::BuildFlatInput()
{
    if (bInputBuilt)
    {
        return;
    }

    auto MakeAxis = [this](const TCHAR* Name) -> UInputAction*
    {
        UInputAction* Action = NewObject<UInputAction>(this, Name);
        Action->ValueType = EInputActionValueType::Axis1D;
        return Action;
    };

    MoveForwardAction = MakeAxis(TEXT("IA_Flat_MoveForward"));
    MoveRightAction = MakeAxis(TEXT("IA_Flat_MoveRight"));
    TurnAction = MakeAxis(TEXT("IA_Flat_Turn"));
    LookUpAction = MakeAxis(TEXT("IA_Flat_LookUp"));

    auto MakeButton = [this](const TCHAR* Name) -> UInputAction*
    {
        UInputAction* Action = NewObject<UInputAction>(this, Name);
        Action->ValueType = EInputActionValueType::Boolean;
        return Action;
    };

    JumpAction = MakeButton(TEXT("IA_Flat_Jump"));
    InteractAction = MakeButton(TEXT("IA_Flat_Interact"));
    RunAction = MakeButton(TEXT("IA_Flat_Run"));
    MeditateAction = MakeButton(TEXT("IA_Flat_Meditate"));

    FlatContext = NewObject<UInputMappingContext>(this, TEXT("IMC_FlatPlayer"));

    auto Map = [this](UInputAction* Action, const FKey& Key, bool bNegate) -> FEnhancedActionKeyMapping&
    {
        FEnhancedActionKeyMapping& Mapping = FlatContext->MapKey(Action, Key);
        if (bNegate)
        {
            Mapping.Modifiers.Add(NewObject<UInputModifierNegate>(this));
        }
        return Mapping;
    };

    Map(MoveForwardAction, EKeys::W, false);
    Map(MoveForwardAction, EKeys::S, true);
    Map(MoveForwardAction, EKeys::Up, false);
    Map(MoveForwardAction, EKeys::Down, true);

    Map(MoveRightAction, EKeys::D, false);
    Map(MoveRightAction, EKeys::A, true);
    Map(MoveRightAction, EKeys::Right, false);
    Map(MoveRightAction, EKeys::Left, true);

    Map(TurnAction, EKeys::MouseX, false);
    Map(LookUpAction, EKeys::MouseY, true);

    Map(JumpAction, EKeys::SpaceBar, false);
    Map(InteractAction, EKeys::E, false);
    Map(RunAction, EKeys::LeftShift, false);
    Map(MeditateAction, EKeys::M, false);

    bInputBuilt = true;
}

void AHoneycombPlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    BuildFlatInput();

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EIC->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &AHoneycombPlayerPawn::OnMoveForward);
        EIC->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &AHoneycombPlayerPawn::OnMoveRight);
        EIC->BindAction(TurnAction, ETriggerEvent::Triggered, this, &AHoneycombPlayerPawn::OnTurn);
        EIC->BindAction(LookUpAction, ETriggerEvent::Triggered, this, &AHoneycombPlayerPawn::OnLookUp);
        EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
        EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
        EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &AHoneycombPlayerPawn::OnInteractPressed);
        EIC->BindAction(RunAction, ETriggerEvent::Started, this, &AHoneycombPlayerPawn::OnRunStarted);
        EIC->BindAction(RunAction, ETriggerEvent::Completed, this, &AHoneycombPlayerPawn::OnRunStopped);
        EIC->BindAction(MeditateAction, ETriggerEvent::Started, this, &AHoneycombPlayerPawn::OnMeditatePressed);
    }

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
                ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(FlatContext, 0);
        }
    }
}

void AHoneycombPlayerPawn::OnMoveForward(const FInputActionValue& Value)
{
    if (Controller && Value.Get<float>() != 0.0f)
    {
        const FRotator YawOnly(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
        AddMovementInput(FRotationMatrix(YawOnly).GetUnitAxis(EAxis::X), Value.Get<float>());
    }
}

void AHoneycombPlayerPawn::OnMoveRight(const FInputActionValue& Value)
{
    if (Controller && Value.Get<float>() != 0.0f)
    {
        const FRotator YawOnly(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
        AddMovementInput(FRotationMatrix(YawOnly).GetUnitAxis(EAxis::Y), Value.Get<float>());
    }
}

void AHoneycombPlayerPawn::OnTurn(const FInputActionValue& Value)
{
    AddControllerYawInput(Value.Get<float>());
}

void AHoneycombPlayerPawn::OnLookUp(const FInputActionValue& Value)
{
    AddControllerPitchInput(Value.Get<float>());
}

void AHoneycombPlayerPawn::OnRunStarted()
{
    SetRunHeld(true);
}

void AHoneycombPlayerPawn::OnRunStopped()
{
    SetRunHeld(false);
}

void AHoneycombPlayerPawn::OnInteractPressed()
{
    if (bInteracting)
    {
        EndInteract();
    }
    else
    {
        BeginInteract();
    }
}

void AHoneycombPlayerPawn::OnMeditatePressed()
{
    AGameManager* GM = AGameManager::GetInstance();
    if (!GM)
    {
        return;
    }

    UKriyaYogaSystem* Kriya = GM->GetKriyaYogaSystem();
    if (!Kriya)
    {
        return;
    }

    AVRUIManager* UI = AVRUIManager::GetInstance(GetWorld());

    if (bMeditating || Kriya->IsKriyaActive())
    {
        Kriya->StopKriyaPractice();
        bMeditating = false;
        if (UI)
        {
            UI->HideMeditationProgress();
        }
        return;
    }

    // Practise the next chakra that is unlocked and not yet activated, which is
    // exactly the one whose activation advances GameManager.
    int32 Target = 0;
    for (int32 Level = 1; Level <= 12; ++Level)
    {
        if (Kriya->IsKriyaUnlocked(Level) && !Kriya->IsChakraActivated(Level))
        {
            Target = Level;
            break;
        }
    }

    if (Target == 0)
    {
        Target = FMath::Clamp(GM->GetCurrentChakraLevel(), 1, 12);
    }

    Kriya->StartKriyaPractice(Target);

    if (Kriya->IsKriyaActive())
    {
        bMeditating = true;
        if (UI)
        {
            UI->ShowMeditationProgress(Kriya->GetKriyaProgress());
        }
    }
}

void AHoneycombPlayerPawn::SetRunHeld(bool bHeld)
{
    bRunHeld = bHeld;
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = bRunHeld ? RunSpeed : WalkSpeed;
    }
}

void AHoneycombPlayerPawn::BeginInteract()
{
    bInteracting = true;
    UpdateFocus();
    UE_LOG(LogTemp, Log, TEXT("[Honeycomb] Interact begin; focused=%s"),
        FocusedActor ? *FocusedActor->GetName() : TEXT("none"));
}

void AHoneycombPlayerPawn::EndInteract()
{
    bInteracting = false;
}

void AHoneycombPlayerPawn::UpdateFocus()
{
    if (!FirstPersonCamera || !GetWorld())
    {
        FocusedActor = nullptr;
        return;
    }

    const FVector Start = FirstPersonCamera->GetComponentLocation();
    const FVector End = Start + FirstPersonCamera->GetForwardVector() * InteractRange;

    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(HoneycombFlatFocus), false, this);
    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
    {
        FocusedActor = Hit.GetActor();
    }
    else
    {
        FocusedActor = nullptr;
    }
}
