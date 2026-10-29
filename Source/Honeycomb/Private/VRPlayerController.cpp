#include "VRPlayerController.h"
#include "Camera/CameraComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "GameManager.h"
#include "VRUIManager.h"
#include "Systems/ChakraSystem.h"
#include "Systems/KarmaSystem.h"
#include "Systems/MoodSystem.h"
#include "Systems/QuestSystem.h"
#include "Systems/DiseaseSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"

AVRPlayerController::AVRPlayerController()
{
    PrimaryActorTick.bCanEverTick = true;

    VRCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VRCamera"));
    VRCamera->SetupAttachment(RootComponent);

    TeleportComponent = CreateDefaultSubobject<USphereComponent>(TEXT("TeleportComponent"));
    TeleportComponent->SetupAttachment(RootComponent);
    TeleportComponent->SetHiddenInGame(true);
    TeleportComponent->SetVisibility(false);

    TeleportVisualizer = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("TeleportVisualizer"));
    TeleportVisualizer->SetupAttachment(RootComponent);
    TeleportVisualizer->bAutoActivate = false;

    MeditationParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("MeditationParticles"));
    MeditationParticles->SetupAttachment(RootComponent);
    MeditationParticles->bAutoActivate = false;

    DeepMeditationSound = CreateDefaultSubobject<UAudioComponent>(TEXT("DeepMeditationSound"));
    DeepMeditationSound->bAutoActivate = false;
}

void AVRPlayerController::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth;
    IsRunning = false;
    bIsMeditating = false;
    bHighMeditationState = false;
    MeditationProgress = 0.0f;

    InitializeSystems();
    InitializePlayer();
    SetupInputActions();

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        if (InputMappingContext)
        {
            Subsystem->AddMappingContext(InputMappingContext, 0);
        }
    }
}

void AVRPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    HandleMeditation(DeltaTime);
    UpdateUI();
}

void AVRPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    UInputComponent* PlayerInputComponent = InputComponent;
    if (!PlayerInputComponent) return;

    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (MeditateAction)
            EnhancedInput->BindAction(MeditateAction, ETriggerEvent::Started, this, &AVRPlayerController::OnMeditate);

        if (HealAction)
            EnhancedInput->BindAction(HealAction, ETriggerEvent::Started, this, &AVRPlayerController::OnHeal);

        if (TeleportAction)
            EnhancedInput->BindAction(TeleportAction, ETriggerEvent::Started, this, &AVRPlayerController::OnTeleport);

        if (ToggleRunAction)
            EnhancedInput->BindAction(ToggleRunAction, ETriggerEvent::Started, this, &AVRPlayerController::OnToggleRun);
    }

    if (bEnableKeyboardFallbacks)
    {
        PlayerInputComponent->BindKey(MeditateKey, IE_Pressed, this, &AVRPlayerController::OnMeditate);
        PlayerInputComponent->BindKey(HealKey, IE_Pressed, this, &AVRPlayerController::OnHeal);
        PlayerInputComponent->BindKey(TeleportKey, IE_Pressed, this, &AVRPlayerController::OnTeleport);
        PlayerInputComponent->BindKey(ToggleRunKey, IE_Pressed, this, &AVRPlayerController::OnToggleRun);
    }
}

void AVRPlayerController::OnMeditate()
{
    ToggleMeditation();
}

void AVRPlayerController::OnHeal()
{
    UseHealing();
}

void AVRPlayerController::OnTeleport()
{
    UseTeleport();
}

void AVRPlayerController::OnToggleRun()
{
    ToggleRun();
}

void AVRPlayerController::InitializeSystems()
{
    GameManager = Cast<AGameManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameManager::StaticClass()));

    if (GameManager)
    {
        ChakraSystem = GameManager->GetChakraSystem();
        QuestSystem = GameManager->GetQuestSystem();
        KarmaSystem = GameManager->GetKarmaSystem();
        MoodSystem = GameManager->GetMoodSystem();
        DiseaseSystem = GameManager->GetDiseaseSystem();
    }

    VRUIManager = Cast<AVRUIManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AVRUIManager::StaticClass()));
}

void AVRPlayerController::InitializePlayer()
{
    CurrentHealth = MaxHealth;

    if (!UnlockedAbilities.Contains("BasicMovement"))
        UnlockedAbilities.Add("BasicMovement");

    if (!UnlockedAbilities.Contains("BasicInteraction"))
        UnlockedAbilities.Add("BasicInteraction");

    UE_LOG(LogTemp, Log, TEXT("Player initialized with basic abilities"));
}

void AVRPlayerController::SetupInputActions()
{
    UE_LOG(LogTemp, Log, TEXT("Setup input actions..."));
    CacheTeleportAction();
}

void AVRPlayerController::CacheTeleportAction()
{
    CachedTeleportAction = TeleportAction;
}

void AVRPlayerController::SetupProgrammaticTeleportAction()
{
    UE_LOG(LogTemp, Log, TEXT("Setting up programmatic teleport action..."));

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        TeleportAction = NewObject<UInputAction>(this);
        TeleportAction->ValueType = EInputActionValueType::Boolean;
    }
}

void AVRPlayerController::EnableTeleportation()
{
    UE_LOG(LogTemp, Log, TEXT("Enabling teleportation..."));

    if (!TeleportAction)
    {
        SetupProgrammaticTeleportAction();
    }

    EnableTeleportationComponents();
    UE_LOG(LogTemp, Log, TEXT("Teleportation enabled"));
}

void AVRPlayerController::DisableTeleportation()
{
    UE_LOG(LogTemp, Log, TEXT("Disabling teleportation..."));
}

void AVRPlayerController::EnableTeleportationComponents()
{
    if (TeleportComponent)
    {
        TeleportComponent->SetComponentTickEnabled(true);
        TeleportComponent->Activate();
    }
    if (TeleportVisualizer)
    {
        TeleportVisualizer->SetVisibility(true);
    }
}

void AVRPlayerController::ActivateTeleportationFallback()
{
    UE_LOG(LogTemp, Warning, TEXT("Activating teleportation fallback (keyboard)"));
}

void AVRPlayerController::UseTeleport()
{
    if (!ChakraSystem || !ChakraSystem->CanUseAbility(30.0f) || !VRCamera)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot teleport: not enough energy or missing components"));
        return;
    }

    FVector CameraForward = VRCamera->GetForwardVector();
    CameraForward.Z = 0.0f;
    CameraForward.Normalize();

    FVector StartLocation = static_cast<AActor*>(this)->GetActorLocation() + CameraForward * TeleportRange;
    StartLocation.Z += 300.0f;

    FVector EndLocation = StartLocation - FVector(0, 0, 500.0f);

    FHitResult HitResult;
    FCollisionQueryParams CollisionParams;
    CollisionParams.bTraceComplex = true;

    if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, CollisionParams))
    {
        ChakraSystem->UseEnergy(30.0f);
        static_cast<AActor*>(this)->SetActorLocation(HitResult.Location + FVector(0, 0, 100.0f));
        UE_LOG(LogTemp, Log, TEXT("Teleportation used"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid teleport location"));
    }
}

void AVRPlayerController::UseHealing()
{
    if (!ChakraSystem || !ChakraSystem->CanUseAbility(20.0f))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot heal: not enough energy or missing system"));
        return;
    }

    ChakraSystem->UseEnergy(20.0f);
    CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + HealingPower);

    if (KarmaSystem)
    {
        KarmaSystem->AddKarmaForAction(KarmaAction::HealSelf);
    }

    UE_LOG(LogTemp, Log, TEXT("Healing ability used"));
    if (QuestSystem) QuestSystem->UpdateObjective("EMO_001", 0, 1);
}

void AVRPlayerController::ToggleMeditation()
{
    if (bIsMeditating)
        StopMeditation();
    else
        StartMeditation();
}

void AVRPlayerController::StartMeditation()
{
    if (bIsMeditating) return;

    bIsMeditating = true;
    MeditationProgress = 0.0f;

    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        Movement->MaxWalkSpeed = WalkSpeed * 0.3f;
    }

    UE_LOG(LogTemp, Log, TEXT("Meditation started..."));

    if (KarmaSystem)
    {
        KarmaSystem->AddKarmaForAction(KarmaAction::Meditation);
    }
    if (MoodSystem)
    {
        MoodSystem->AddCalm(0.1f);
        MoodSystem->AddLove(0.05f);
    }
    if (QuestSystem)
        QuestSystem->UpdateObjective("MEN_002", 0, 1);
}

void AVRPlayerController::StopMeditation()
{
    if (!bIsMeditating) return;

    bIsMeditating = false;

    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        Movement->MaxWalkSpeed = IsRunning ? RunSpeed : WalkSpeed;
    }

    UE_LOG(LogTemp, Log, TEXT("Meditation stopped"));
}

void AVRPlayerController::CompleteMeditation()
{
    bIsMeditating = false;
    MeditationProgress = 0.0f;

    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        Movement->MaxWalkSpeed = IsRunning ? RunSpeed : WalkSpeed;
    }

    if (ChakraSystem)
    {
        ChakraSystem->AddEnergy(ChakraSystem->GetMaxEnergy());
    }

    CurrentHealth = MaxHealth;
    UnlockAbility("MeditationMastery");

    UE_LOG(LogTemp, Log, TEXT("Meditation completed! New abilities unlocked."));
    if (QuestSystem)
        QuestSystem->UpdateObjective("MEN_002", 0, 1);
}

void AVRPlayerController::HandleMeditation(float DeltaTime)
{
    if (!bIsMeditating) return;

    MeditationProgress += DeltaTime * 10.0f;

    if (MeditationProgress >= MeditationRequired)
    {
        CompleteMeditation();
        return;
    }

    if (ChakraSystem)
    {
        ChakraSystem->AddEnergy(DeltaTime * 5.0f);
    }

    CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + DeltaTime * HealthRegeneration);

    if (MoodSystem)
        MoodSystem->ApplyMeditationBoost();
}

void AVRPlayerController::UnlockAbility(const FString& AbilityName)
{
    if (AbilityName.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Attempted to unlock ability with empty name"));
        return;
    }

    if (UnlockedAbilities.Contains(AbilityName))
    {
        UE_LOG(LogTemp, Log, TEXT("Ability %s is already unlocked"), *AbilityName);
        return;
    }

    UnlockedAbilities.Add(AbilityName);

    if (AbilityName == "MeditationMastery")
    {
        MeditationRequired *= 0.8f;
        HealingPower *= 1.5f;
        UE_LOG(LogTemp, Log, TEXT("MeditationMastery unlocked! Benefits applied."));
    }
    else if (AbilityName == "ChakraVision")
    {
        UE_LOG(LogTemp, Log, TEXT("ChakraVision unlocked!"));
    }
    else if (AbilityName == "DimensionalShift")
    {
        TeleportRange *= 2.0f;
        UE_LOG(LogTemp, Log, TEXT("DimensionalShift unlocked!"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("New ability unlocked: %s"), *AbilityName);
    }

    if (VRUIManager)
    {
        VRUIManager->ShowNotification(AbilityName);
    }
}

void AVRPlayerController::ToggleRun()
{
    IsRunning = !IsRunning;

    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        Movement->MaxWalkSpeed = IsRunning ? RunSpeed : WalkSpeed;
    }

    UE_LOG(LogTemp, Log, TEXT("Run mode: %s, Speed: %f"), IsRunning ? TEXT("ON") : TEXT("OFF"),
        (GetCharacterMovement() != nullptr) ? GetCharacterMovement()->MaxWalkSpeed : 0.0f);
}

void AVRPlayerController::ApplyPlayerDamage(float Damage)
{
    CurrentHealth -= Damage;
    UE_LOG(LogTemp, Warning, TEXT("Player took %.1f damage, remaining health: %.1f"), Damage, CurrentHealth);

    if (MoodSystem)
        MoodSystem->ApplyDamageShock(Damage);

    if (CurrentHealth <= 0.0f)
    {
        Die();
    }
}

void AVRPlayerController::Heal(float Amount)
{
    CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + Amount);
    UE_LOG(LogTemp, Log, TEXT("Player healed by %.1f, current health: %.1f"), Amount, CurrentHealth);
}

void AVRPlayerController::Die()
{
    UE_LOG(LogTemp, Log, TEXT("Player died. Transitioning to another world..."));
    if (GameManager)
        GameManager->OnPlayerDeath();
}

void AVRPlayerController::UpdateUI()
{
    if (!VRUIManager) return;

    VRUIManager->UpdateHealthBar(GetHealthPercentage());

    if (ChakraSystem)
    {
        float EnergyPercent = ChakraSystem->GetEnergyPercentage();
        VRUIManager->UpdateEnergyBar(EnergyPercent);
        VRUIManager->SetEnergyBarVisible(bIsMeditating || EnergyPercent < 0.95f);
    }

    if (bIsMeditating)
    {
        float MeditationPercent = GetMeditationProgress();
        VRUIManager->ShowMeditationProgress(MeditationPercent);
        UpdateMeditationVFX(MeditationPercent);
    }
    else
    {
        VRUIManager->HideMeditationProgress();
    }

    PositionVRUI();
}

void AVRPlayerController::PositionVRUI()
{
    if (!VRUIManager || !VRCamera) return;

    FVector UIPosition = VRCamera->GetComponentLocation() + VRCamera->GetForwardVector() * 200.0f;
    UIPosition.Z -= 30.0f;
    VRUIManager->SetActorLocation(UIPosition);
    VRUIManager->SetActorRotation(FRotationMatrix::MakeFromX(VRUIManager->GetActorLocation() - VRCamera->GetComponentLocation()).Rotator());
}

void AVRPlayerController::UpdateMeditationVFX(float Progress)
{
    float Intensity = FMath::Clamp(Progress * 2.0f, 0.0f, 1.0f);

    if (MeditationParticles)
    {
        MeditationParticles->SetFloatParameter("Intensity", Intensity);
    }

    if (Progress > 0.7f && !bHighMeditationState)
    {
        bHighMeditationState = true;
        OnHighMeditationStateEnter();
    }
    else if (Progress <= 0.7f && bHighMeditationState)
    {
        bHighMeditationState = false;
        OnHighMeditationStateExit();
    }
}

void AVRPlayerController::OnHighMeditationStateEnter()
{
    if (DeepMeditationSound)
        DeepMeditationSound->Play();

    HealthRegeneration *= 2.0f;

    if (ChakraSystem)
    {
        ChakraSystem->SetEnergyRegenerationRate(ChakraSystem->GetEnergyRegenerationRate() * 2.0f);
    }

    if (KarmaSystem)
    {
        KarmaSystem->AddKarmaForAction(KarmaAction::DeepMeditation);
    }

    if (QuestSystem)
        QuestSystem->UpdateObjective("MEN_003", 0, 1);
}

void AVRPlayerController::OnHighMeditationStateExit()
{
    HealthRegeneration /= 2.0f;

    if (ChakraSystem)
    {
        ChakraSystem->SetEnergyRegenerationRate(ChakraSystem->GetEnergyRegenerationRate() / 2.0f);
    }

    if (DeepMeditationSound && DeepMeditationSound->IsPlaying())
        DeepMeditationSound->Stop();
}

float AVRPlayerController::GetHealthPercentage() const
{
    return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
}

float AVRPlayerController::GetMeditationProgress() const
{
    return MeditationRequired > 0.0f ? MeditationProgress / MeditationRequired : 0.0f;
}

float AVRPlayerController::GetMoveSpeed() const
{
    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        return Movement->MaxWalkSpeed;
    }
    return WalkSpeed;
}

void AVRPlayerController::SetMoveSpeedMultiplier(float Multiplier)
{
    MoveSpeedMultiplier = Multiplier;

    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        if (BaseMaxWalkSpeed <= 0.0f)
        {
            BaseMaxWalkSpeed = Movement->MaxWalkSpeed;
        }
        Movement->MaxWalkSpeed = BaseMaxWalkSpeed * MoveSpeedMultiplier;
    }
}

void AVRPlayerController::SetTurnEnabled(bool bEnable)
{
    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        Movement->RotationRate = bEnable ? FRotator(0.0f, 540.0f, 0.0f) : FRotator::ZeroRotator;
    }
}

UCharacterMovementComponent* AVRPlayerController::GetCharacterMovement() const
{
    if (APawn* ControlledPawn = GetPawn())
    {
        if (ACharacter* ControlledCharacter = Cast<ACharacter>(ControlledPawn))
        {
            return ControlledCharacter->GetCharacterMovement();
        }
    }
    return nullptr;
}
