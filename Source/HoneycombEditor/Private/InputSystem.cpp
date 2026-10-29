#include "InputSystem.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "JsonObjectConverter.h"
#include "Engine/Engine.h"

#if WITH_EDITOR
    #include "ISettingsModule.h"
    #include "ISettingsSection.h"
    #include "Framework/Notifications/NotificationManager.h"
    #include "Widgets/Notifications/SNotificationList.h"
    #include "Editor.h"
    #include "Settings/ProjectPackagingSettings.h"
#endif

void UInputSystem::SetInputHandlingBoth()
{
    #if WITH_EDITOR
        SetInputHandlingMode(TEXT("Both"));
    #endif
}

void UInputSystem::SetInputHandlingLegacy()
{
    #if WITH_EDITOR
        SetInputHandlingMode(TEXT("Legacy"));
    #endif
}

void UInputSystem::SetInputHandlingEnhanced()
{
    #if WITH_EDITOR
        SetInputHandlingMode(TEXT("Enhanced"));
    #endif
}

void UInputSystem::SetInputHandlingMode(const FString& Mode)
{
    #if WITH_EDITOR
        FString ConfigDir = FPaths::ProjectConfigDir();
        FString InputConfigPath = ConfigDir / TEXT("DefaultInput.ini");
        
        if (FPaths::FileExists(InputConfigPath))
        {
            FString FileContent;
            FFileHelper::LoadFileToString(FileContent, *InputConfigPath);
            if (Mode == TEXT("Both"))
            {
                FileContent.ReplaceInline(TEXT("bUseEnhancedInput=true"), TEXT("bUseEnhancedInput=true"));
                FileContent.ReplaceInline(TEXT("bUseLegacyInput=true"), TEXT("bUseLegacyInput=true"));
            }
            else if (Mode == TEXT("Legacy"))
            {
                FileContent.ReplaceInline(TEXT("bUseEnhancedInput=true"), TEXT("bUseEnhancedInput=false"));
                FileContent.ReplaceInline(TEXT("bUseLegacyInput=false"), TEXT("bUseLegacyInput=true"));
            }
            else if (Mode == TEXT("Enhanced"))
            {
                FileContent.ReplaceInline(TEXT("bUseEnhancedInput=false"), TEXT("bUseEnhancedInput=true"));
                FileContent.ReplaceInline(TEXT("bUseLegacyInput=true"), TEXT("bUseLegacyInput=false"));
            }
            
            FFileHelper::SaveStringToFile(FileContent, *InputConfigPath);
        }
        else
        {
            FString NewContent = FString::Printf(TEXT(
                "[/Script/Engine.InputSettings]\n"
                "bUseEnhancedInput=%s\n"
                "bUseLegacyInput=%s\n"
            ), (Mode == TEXT("Enhanced") || Mode == TEXT("Both")) ? TEXT("true") : TEXT("false"),
                (Mode == TEXT("Legacy") || Mode == TEXT("Both")) ? TEXT("true") : TEXT("false")
            );
            
            FFileHelper::SaveStringToFile(NewContent, *InputConfigPath);
        }
        
        UpdateProjectSettings(TEXT("bUseEnhancedInput"), (Mode == TEXT("Enhanced") || Mode == TEXT("Both")) ? TEXT("true") : TEXT("false"));
        
        SaveProjectSettings();
        ShowNotification(FString::Printf(TEXT("Input Handling set to: %s"), *Mode));
    #endif
}

void UInputSystem::UpdateProjectSettings(const FString& SettingName, const FString& Value)
{
    #if WITH_EDITOR
        if (!GConfig) return;

        const FString Section = TEXT("/Script/Engine.InputSettings");
        
        GConfig->SetString(*Section, *SettingName, *Value, GInputIni);
        GConfig->Flush(false, GInputIni);

        UE_LOG(LogTemp, Log, TEXT("UInputSystem: Updated %s = %s"), *SettingName, *Value);
    #else
        UE_LOG(LogTemp, Warning, TEXT("UInputSystem: UpdateProjectSettings called outside editor"));
    #endif
}

void UInputSystem::SaveProjectSettings()
{
    #if WITH_EDITOR
        if (GConfig)
        {
            GConfig->Flush(false, GInputIni);
        }

        UE_LOG(LogTemp, Log, TEXT("UInputSystem: Project settings saved"));
    #endif
}

void UInputSystem::ShowNotification(const FString& Message)
{
    #if WITH_EDITOR
        FNotificationInfo Info(FText::FromString(Message));
        Info.bFireAndForget = true;
        Info.ExpireDuration = 3.0f;
        Info.bUseSuccessFailIcons = true;
        Info.Image = FAppStyle::GetBrush(TEXT("NotificationList.SuccessImage"));
        
        FSlateNotificationManager::Get().AddNotification(Info);
    #endif
}

FString UInputSystem::GetCurrentInputHandling()
{
    FString Result = TEXT("Unknown");
    
    #if WITH_EDITOR
        FString InputConfigPath = FPaths::ProjectConfigDir() / TEXT("DefaultInput.ini");
        
        if (FPaths::FileExists(InputConfigPath))
        {
            FString FileContent;
            FFileHelper::LoadFileToString(FileContent, *InputConfigPath);
            
            bool bEnhanced = FileContent.Contains(TEXT("bUseEnhancedInput=true"));
            bool bLegacy = FileContent.Contains(TEXT("bUseLegacyInput=true"));
            
            if (bEnhanced && bLegacy)
            {
                Result = TEXT("Both");
            }
            else if (bEnhanced)
            {
                Result = TEXT("Enhanced");
            }
            else if (bLegacy)
            {
                Result = TEXT("Legacy");
            }
        }
    #endif
    
    return Result;
}
