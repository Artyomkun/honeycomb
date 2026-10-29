#include "DummyEditorScript.h"
#include "HoneycombCore.h"
#include "Editor.h"
#include "Misc/MessageDialog.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

IMPLEMENT_MODULE(DummyEditorScript, HoneycombEditor)

void DummyEditorScript::StartupModule()
{
    OnEditorStartup();
    BindEditorDelegates();
}

void DummyEditorScript::ShutdownModule()
{
    OnEditorShutdown();
}

void DummyEditorScript::OnEditorStartup()
{
    UE_LOG(LogTemp, Log, TEXT("Editor started - DummyEditorScript analog"));
    FNotificationInfo Info(FText::FromString(TEXT("Editor Module Loaded!")));
    Info.bFireAndForget = true;
    Info.ExpireDuration = 2.0f;
    FSlateNotificationManager::Get().AddNotification(Info);
}

void DummyEditorScript::OnEditorShutdown()
{
    UE_LOG(LogTemp, Log, TEXT("Editor shutting down"));
}

void DummyEditorScript::BindEditorDelegates()
{
    if (GEditor)
    {
        FCoreDelegates::OnExit.AddRaw(this, &DummyEditorScript::OnEditorShutdown);
    }
}