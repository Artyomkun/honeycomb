#include "MeditationSystem.h"
#include "VRPlayerController.h"
#include "ChakraSystem.h"
#include "VRUIManager.h"
#include "GameManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"

namespace MeditationAbilities
{
    static const FString BasicMeditation = TEXT("BasicMeditation");
    static const FString DeepMeditation = TEXT("DeepMeditation");
    static const FString AstralProjection = TEXT("AstralProjection");
}

UMeditationSystem::UMeditationSystem()
{
    bIsMeditating = false;
    CurrentMeditationLevel = 0;
    MeditationProgress = 0.0f;
    PlayerController = nullptr;
    UE_LOG(LogTemp, Verbose, TEXT("UMeditationSystem: Constructor initialized"));
}

void UMeditationSystem::Initialize()
{
    if (bIsInitialized) return;

    bIsMeditating = false;
    CurrentMeditationLevel = 0;
    MeditationProgress = 0.0f;
    MeditationTime = 0.0f;

    if (!MeditationAudioComponent)
    {
        if (USoundBase* Sound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/Meditation.Meditation")))
        {
            if (UWorld* World = GetWorld())
            {
                MeditationAudioComponent = UGameplayStatics::SpawnSound2D(World, Sound, 1.0f, 1.0f, 0.0f, nullptr, false, false);
            }
        }
    }

    bIsInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("UMeditationSystem initialized"));
}

void UMeditationSystem::Update(float DeltaTime)
{
    if (!bIsInitialized) return;

    if (NotificationTimer > 0.0f)
    {
        NotificationTimer -= DeltaTime;
        if (NotificationTimer <= 0.0f)
        {
            CurrentNotification.Empty();
        }
    }

    if (bIsMeditating)
    {
        float Speed = CalculateMeditationSpeed();
        MeditationTime += DeltaTime;
        MeditationProgress += Speed * DeltaTime;
        MeditationProgress = FMath::Clamp(MeditationProgress, 0.0f, 1.0f);

        UpdateMeditationUI(MeditationProgress);

        if (MeditationProgress >= 1.0f)
        {
            CompleteMeditation();
        }
    }
}

void UMeditationSystem::StartMeditation(int32 Level)
{
    if (bIsMeditating) return;

    CurrentMeditationLevel = Level;
    bIsMeditating = true;
    MeditationProgress = 0.0f;
    MeditationTime = 0.0f;

    if (PlayerController)
    {
        OriginalMoveSpeed = PlayerController->GetMoveSpeed();
    }

    ApplyMeditationEffects(true);
    ShowNotification(FString::Printf(TEXT("Начата медитация на чакре %d"), Level));

    UE_LOG(LogTemp, Log, TEXT("Meditation started at level %d"), Level);
}

void UMeditationSystem::StopMeditation()
{
    if (!bIsMeditating) return;

    bIsMeditating = false;

    ApplyMeditationEffects(false);
    ApplyMeditationRewards();

    UE_LOG(LogTemp, Log, TEXT("Meditation stopped. Progress: %.1f%%"), MeditationProgress * 100.0f);
}

void UMeditationSystem::SetVRInput(bool bStartPressed, bool bStopPressed)
{
    if (bStartPressed && !bIsMeditating)
    {
        AGameManager* GameManager = Cast<AGameManager>(
            UGameplayStatics::GetActorOfClass(GetWorld(), AGameManager::StaticClass())
        );

        if (!GameManager)
        {
            UE_LOG(LogTemp, Warning, TEXT("UMeditationSystem: GameManager not found"));
            return;
        }

        int32 Level = GameManager->GetMeditationLevel();

        if (Level <= 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("UMeditationSystem: Invalid meditation level: %d"), Level);
            return;
        }

        StartMeditation(Level);
        ShowNotification(FString::Printf(TEXT("Медитация начата (уровень %d)"), Level));
    }

    if (bStopPressed && bIsMeditating)
    {
        StopMeditation();
        ShowNotification(TEXT("Медитация остановлена"));
    }
}

float UMeditationSystem::CalculateMeditationSpeed() const
{
    float Speed = BaseMeditationSpeed;

    Speed *= 1.0f + CurrentMeditationLevel * 0.2f;

    if (ChakraSystemRef)
    {
        Speed *= 1.0f + ChakraSystemRef->GetCurrentChakraLevel() * 0.1f;
    }

    Speed *= GetEnvironmentMeditationBonus();

    return Speed;
}

float UMeditationSystem::GetEnvironmentMeditationBonus() const
{
    float Bonus = 1.0f;

    if (IsInMeditationZone())
    {
        Bonus *= 1.5f;
    }

    if (IsNightTime())
    {
        Bonus *= 1.2f;
    }

    return Bonus;
}

bool UMeditationSystem::IsInMeditationZone() const
{
    return false;
}

bool UMeditationSystem::IsNightTime() const
{
    return false;
}

void UMeditationSystem::ApplyMeditationEffects(bool bStarting)
{
    if (!PlayerController) return;

    if (bStarting)
    {
        PlayerController->SetMoveSpeedMultiplier(0.1f);
        PlayerController->SetTurnEnabled(false);
        EnableMeditationVFX(true);
        PlayMeditationSound(true);
    }
    else
    {
        PlayerController->SetMoveSpeedMultiplier(1.0f);
        PlayerController->SetTurnEnabled(true);
        EnableMeditationVFX(false);
        PlayMeditationSound(false);
    }
}

void UMeditationSystem::ApplyMeditationRewards()
{
    if (MeditationProgress < 0.3f) return;

    float EnergyGain = MeditationProgress * 50.0f * (1.0f + CurrentMeditationLevel * 0.2f);

    if (ChakraSystemRef)
    {
        ChakraSystemRef->AddEnergy(EnergyGain);
    }

    if (MeditationProgress >= 1.0f)
    {
        CompleteMeditationRewards();
    }
}

void UMeditationSystem::CompleteMeditationRewards()
{
    UE_LOG(LogTemp, Log, TEXT("Meditation completed! Special rewards."));

    if (ChakraSystemRef)
    {
        ChakraSystemRef->SetMaxEnergy(ChakraSystemRef->GetMaxEnergy() * 1.01f);
    }

    UnlockMeditationAbility();
}

void UMeditationSystem::CompleteMeditation()
{
    UE_LOG(LogTemp, Log, TEXT("Meditation level %d fully completed!"), CurrentMeditationLevel);

    PlayCompletionEffects();
    StopMeditation();
}

void UMeditationSystem::UpdateMeditationUI(float Progress)
{
    if (VRUIManagerRef)
    {
        VRUIManagerRef->ShowMeditationProgress(Progress);
    }
}

void UMeditationSystem::EnableMeditationVFX(bool bEnable)
{
    if (!VRUIManagerRef)
    {
        UE_LOG(LogTemp, Warning, TEXT("UMeditationSystem: VRUIManager not found"));
        return;
    }

    if (bEnable)
    {
        VRUIManagerRef->ShowMeditationVFX();
        UE_LOG(LogTemp, Log, TEXT("UMeditationSystem: Meditation VFX enabled"));
    }
    else
    {
        VRUIManagerRef->HideMeditationVFX();
        UE_LOG(LogTemp, Log, TEXT("UMeditationSystem: Meditation VFX disabled"));
    }
}

void UMeditationSystem::PlayMeditationSound(bool bPlay)
{
    if (!MeditationAudioComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("UMeditationSystem: AudioComponent not found"));
        return;
    }

    if (bPlay)
    {
        if (!MeditationAudioComponent->IsPlaying())
        {
            MeditationAudioComponent->Play();
            UE_LOG(LogTemp, Log, TEXT("UMeditationSystem: Meditation sound started"));
        }
    }
    else
    {
        if (MeditationAudioComponent->IsPlaying())
        {
            MeditationAudioComponent->Stop();
            UE_LOG(LogTemp, Log, TEXT("UMeditationSystem: Meditation sound stopped"));
        }
    }
}

void UMeditationSystem::PlayCompletionEffects()
{
    EnableMeditationVFX(false);
    PlayMeditationSound(false);
    ShowNotification(TEXT("Медитация завершена"));
    
    if (AGameManager* GM = Cast<AGameManager>(
        UGameplayStatics::GetActorOfClass(GetWorld(), AGameManager::StaticClass())))
    {
        GM->CompleteMeditation(CurrentMeditationLevel);
    }
    
    OnMeditationComplete.Broadcast(CurrentMeditationLevel);
    
    UE_LOG(LogTemp, Log, TEXT("UMeditationSystem: Completion effects played"));
}

void UMeditationSystem::UnlockMeditationAbility()
{
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("UMeditationSystem: PlayerController not found"));
        return;
    }

    if (CurrentMeditationLevel >= 1)
    {
        PlayerController->UnlockAbility(MeditationAbilities::BasicMeditation);
        UE_LOG(LogTemp, Log, TEXT("UMeditationSystem: Unlocked BasicMeditation"));
    }

    if (CurrentMeditationLevel >= 3)
    {
        PlayerController->UnlockAbility(MeditationAbilities::DeepMeditation);
        UE_LOG(LogTemp, Log, TEXT("UMeditationSystem: Unlocked DeepMeditation"));
    }

    if (CurrentMeditationLevel >= 5)
    {
        PlayerController->UnlockAbility(MeditationAbilities::AstralProjection);
        UE_LOG(LogTemp, Log, TEXT("UMeditationSystem: Unlocked AstralProjection"));
    }

    ShowNotification(FString::Printf(TEXT("Медитация: уровень %d. Способности обновлены."), CurrentMeditationLevel));
}

void UMeditationSystem::ShowNotification(const FString& Message)
{
    CurrentNotification = Message;
    NotificationTimer = NotificationDuration;

    if (VRUIManagerRef)
    {
        VRUIManagerRef->ShowNotification(Message);
    }

    UE_LOG(LogTemp, Log, TEXT("[Meditation] %s"), *Message);
}
