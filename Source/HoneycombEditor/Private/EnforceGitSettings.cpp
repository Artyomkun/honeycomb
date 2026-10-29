#include "EnforceGitSettings.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "Engine/Engine.h"

#if WITH_EDITOR
    #include "ISettingsModule.h"
    #include "ISettingsSection.h"
    #include "Framework/Notifications/NotificationManager.h"
    #include "Widgets/Notifications/SNotificationList.h"
    #include "Editor.h"
    #include "UnrealEdGlobals.h"
    #include "Editor/UnrealEdEngine.h"
    #include "Settings/ProjectPackagingSettings.h"
    #include "AssetRegistry/AssetRegistryModule.h"
#endif

void UEnforceGitSettings::EnforceGitSettings()
{
    #if WITH_EDITOR
        SetTextAssetSerialization();
        SetVisibleMetaFiles();
        SaveProjectSettings();
        
        ShowNotification(TEXT("Git Settings Enforced: Text + Visible Meta Files"));
        LogMessage(TEXT("Git settings enforced successfully"));
    #endif
}

void UEnforceGitSettings::SetTextAssetSerialization()
{
    #if WITH_EDITOR
        
        FString ConfigDir = FPaths::ProjectConfigDir();
        FString EditorConfigPath = ConfigDir / TEXT("DefaultEditor.ini");
        
        FString FileContent;
        if (FPaths::FileExists(EditorConfigPath))
        {
            FFileHelper::LoadFileToString(FileContent, *EditorConfigPath);
        }
        
        FString SectionName = TEXT("[/Script/UnrealEd.EditorSettings]");
        FString SettingLine = TEXT("bForceTextAssetSerialization=True");
        
        if (!FileContent.Contains(SettingLine))
        {
            if (FileContent.Contains(SectionName))
            {
                int32 SectionPos = FileContent.Find(SectionName);
                int32 EndPos = FileContent.Find(TEXT("["), ESearchCase::IgnoreCase, ESearchDir::FromEnd, SectionPos + 1);
                if (EndPos == INDEX_NONE)
                {
                    EndPos = FileContent.Len();
                }
                
                FString Before = FileContent.Mid(0, EndPos);
                FString After = FileContent.Mid(EndPos);
                FileContent = Before + TEXT("\n") + SettingLine + TEXT("\n") + After;
            }
            else
            {
                FileContent += TEXT("\n") + SectionName + TEXT("\n") + SettingLine + TEXT("\n");
            }
            
            FFileHelper::SaveStringToFile(FileContent, *EditorConfigPath);
            LogMessage(TEXT("Text asset serialization enabled"));
        }
        else
        {
            LogMessage(TEXT("Text asset serialization already enabled"));
        }
    #endif
}

void UEnforceGitSettings::SetVisibleMetaFiles()
{
    #if WITH_EDITOR
        FString ConfigDir = FPaths::ProjectConfigDir();
        FString EditorConfigPath = ConfigDir / TEXT("DefaultEditor.ini");
        
        FString FileContent;
        if (FPaths::FileExists(EditorConfigPath))
        {
            FFileHelper::LoadFileToString(FileContent, *EditorConfigPath);
        }
        
        FString GitSection = TEXT("[/Script/UnrealEd.EditorSettings]");
        FString GitSetting = TEXT("bShowMetaFilesInContentBrowser=True");
        
        if (!FileContent.Contains(GitSetting))
        {
            if (FileContent.Contains(GitSection))
            {
                int32 SectionPos = FileContent.Find(GitSection);
                int32 EndPos = FileContent.Find(TEXT("["), ESearchCase::IgnoreCase, ESearchDir::FromEnd, SectionPos + 1);
                if (EndPos == INDEX_NONE)
                {
                    EndPos = FileContent.Len();
                }
                
                FString Before = FileContent.Mid(0, EndPos);
                FString After = FileContent.Mid(EndPos);
                FileContent = Before + TEXT("\n") + GitSetting + TEXT("\n") + After;
            }
            else
            {
                FileContent += TEXT("\n") + GitSection + TEXT("\n") + GitSetting + TEXT("\n");
            }
            
            FFileHelper::SaveStringToFile(FileContent, *EditorConfigPath);
            LogMessage(TEXT("Visible meta files enabled"));
        }
        else
        {
            LogMessage(TEXT("Visible meta files already enabled"));
        }
    #endif
}

bool UEnforceGitSettings::AreGitSettingsEnforced()
{
    #if WITH_EDITOR
        FString ConfigDir = FPaths::ProjectConfigDir();
        FString EditorConfigPath = ConfigDir / TEXT("DefaultEditor.ini");
        
        if (FPaths::FileExists(EditorConfigPath))
        {
            FString FileContent;
            FFileHelper::LoadFileToString(FileContent, *EditorConfigPath);
            
            return FileContent.Contains(TEXT("bForceTextAssetSerialization=True")) && FileContent.Contains(TEXT("bShowMetaFilesInContentBrowser=True"));
        }
    #endif

    return false;
}

FString UEnforceGitSettings::GetCurrentAssetSerializationMode()
{
    #if WITH_EDITOR
        FString ConfigDir = FPaths::ProjectConfigDir();
        FString EditorConfigPath = ConfigDir / TEXT("DefaultEditor.ini");
        
        if (FPaths::FileExists(EditorConfigPath))
        {
            FString FileContent;
            FFileHelper::LoadFileToString(FileContent, *EditorConfigPath);
            
            if (FileContent.Contains(TEXT("bForceTextAssetSerialization=True")))
            {
                return TEXT("Text");
            }
            else
            {
                return TEXT("Binary");
            }
        }
    #endif

    return TEXT("Unknown");
}

void UEnforceGitSettings::SaveProjectSettings()
{
    #if WITH_EDITOR
        if (GEditor)
        {
            FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
            AssetRegistryModule.Get().ScanPathsSynchronous({FPaths::ProjectContentDir()}, true);
        }
    #endif
}

void UEnforceGitSettings::ShowNotification(const FString& Message)
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

void UEnforceGitSettings::LogMessage(const FString& Message)
{
    UE_LOG(LogTemp, Log, TEXT("[UEnforceGitSettings] %s"), *Message);
}
