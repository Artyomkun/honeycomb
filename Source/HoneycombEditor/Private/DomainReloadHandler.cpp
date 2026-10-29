#include "DomainReloadHandler.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformStackWalk.h"
#include "Misc/DateTime.h"
#include "Misc/CoreDelegates.h"
#include "Engine/Engine.h"

#if WITH_EDITOR
    #include "Editor.h"
#endif

UDomainReloadHandler* UDomainReloadHandler::Instance = nullptr;

UDomainReloadHandler::UDomainReloadHandler()
{
    #if WITH_EDITOR
        FEditorDelegates::PreBeginPIE.AddUObject(this, &UDomainReloadHandler::OnPreBeginPIE);
        FEditorDelegates::EndPIE.AddUObject(this, &UDomainReloadHandler::OnEndPIE);
    #endif

    FCoreDelegates::OnPreExit.AddUObject(this, &UDomainReloadHandler::OnPreExit);
}

UDomainReloadHandler* UDomainReloadHandler::Get()
{
    if (!Instance)
    {
        Instance = NewObject<UDomainReloadHandler>();
        Instance->AddToRoot();
        Instance->InitializeHandler();
    }
    return Instance;
}

void UDomainReloadHandler::InitializeHandler()
{
    UE_LOG(LogTemp, Log, TEXT("DomainReloadHandler initialized"));
}

void UDomainReloadHandler::RegisterStaticHandle(void* Pointer, const FString& Name)
{
    if (!Pointer) return;

    HandleInfo Info;
    Info.Name = Name;
    Info.Pointer = Pointer;
    Info.AllocationTime = FDateTime::Now();
    Info.bIsAllocated = true;

    #if WITH_EDITOR || UE_BUILD_DEBUG
        constexpr int32 MaxFrames = 32;
        uint64 Frames[MaxFrames];
        const int32 NumFrames = FPlatformStackWalk::CaptureStackBackTrace(Frames, MaxFrames);
        
        ANSICHAR StackTrace[4096];
        FPlatformStackWalk::StackWalkAndDump(StackTrace, sizeof(StackTrace), 0);
        
        UE_LOG(LogTemp, Verbose, TEXT("Stack trace:\n%s"), ANSI_TO_TCHAR(StackTrace));
    #endif

    StaticHandles.Add(Info);

    if (bLogCleanupOperations)
    {
        LogCleanup(FString::Printf(TEXT("Registered static handle: %s"), *Name));
    }
}

void UDomainReloadHandler::UnregisterStaticHandle(void* Pointer)
{
    int32 Removed = StaticHandles.RemoveAll([Pointer](const HandleInfo& Info)
    {
        return Info.Pointer == Pointer;
    });

    if (Removed > 0 && bLogCleanupOperations)
    {
        LogCleanup(TEXT("Unregistered static handle"));
    }
}

void UDomainReloadHandler::RegisterInstanceHandle(void* Pointer, const FString& Name)
{
    if (!Pointer) return;

    HandleInfo Info;
    Info.Name = Name;
    Info.Pointer = Pointer;
    Info.AllocationTime = FDateTime::Now();
    Info.bIsAllocated = true;

    InstanceHandles.Add(Info);

    if (bLogCleanupOperations)
    {
        LogCleanup(FString::Printf(TEXT("Registered instance handle: %s"), *Name));
    }
}

void UDomainReloadHandler::UnregisterInstanceHandle(void* Pointer)
{
    int32 Removed = InstanceHandles.RemoveAll([Pointer](const HandleInfo& Info)
    {
        return Info.Pointer == Pointer;
    });

    if (Removed > 0 && bLogCleanupOperations)
    {
        LogCleanup(TEXT("Unregistered instance handle"));
    }
}

void UDomainReloadHandler::CleanupStaticHandles()
{
    CleanupHandles(StaticHandles, TEXT("static"));
    OnStaticCleanupComplete.Broadcast();
}

void UDomainReloadHandler::CleanupInstanceHandles()
{
    CleanupHandles(InstanceHandles, TEXT("instance"));
    OnInstanceCleanupComplete.Broadcast();
}

void UDomainReloadHandler::CleanupHandles(TArray<HandleInfo>& Handles, const FString& HandleType)
{
    int32 FreedCount = 0;

    for (HandleInfo& Info : Handles)
    {
        if (Info.bIsAllocated && Info.Pointer)
        {
            Info.Pointer = nullptr;
            Info.bIsAllocated = false;
            FreedCount++;

            if (bLogCleanupOperations)
            {
                LogCleanup(FString::Printf(TEXT("Freed %s handle: %s"), *HandleType, *Info.Name));
            }
        }
    }

    Handles.Empty();

    if (bLogCleanupOperations && FreedCount > 0)
    {
        LogCleanup(FString::Printf(TEXT("Domain reload: Freed %d %s handles"), FreedCount, *HandleType));
    }
}

void UDomainReloadHandler::ForceCleanup()
{
    CleanupStaticHandles();
    CleanupInstanceHandles();
    LogCleanup(TEXT("Forced cleanup completed"));
}

void UDomainReloadHandler::LogActiveHandles()
{
    UE_LOG(LogTemp, Log, TEXT("=== Active Handles ==="));
    UE_LOG(LogTemp, Log, TEXT("Static handles: %d"), StaticHandles.Num());
    UE_LOG(LogTemp, Log, TEXT("Instance handles: %d"), InstanceHandles.Num());

    for (const HandleInfo& Info : StaticHandles)
    {
        UE_LOG(LogTemp, Log, TEXT("Static: %s - Allocated: %d - Time: %s"), *Info.Name, Info.bIsAllocated, *Info.AllocationTime.ToString());
    }
}

int32 UDomainReloadHandler::GetStaticHandleCount() const
{
    return StaticHandles.Num();
}

int32 UDomainReloadHandler::GetInstanceHandleCount() const
{
    return InstanceHandles.Num();
}

void UDomainReloadHandler::LogCleanup(const FString& Message)
{
    #if WITH_EDITOR || UE_BUILD_DEBUG
        UE_LOG(LogTemp, Log, TEXT("[DomainReloadHandler] %s"), *Message);
    #endif
}

#if WITH_EDITOR
    void UDomainReloadHandler::OnPreBeginPIE(bool bIsSimulating)
    {
        if (bEnableAutoCleanup)
        {
            CleanupStaticHandles();
        }
    }

    void UDomainReloadHandler::OnEndPIE(bool bIsSimulating)
    {
        if (bEnableAutoCleanup)
        {
            CleanupStaticHandles();
        }
    }
#endif

void UDomainReloadHandler::OnPreExit()
{
    #if WITH_EDITOR
        FEditorDelegates::PreBeginPIE.RemoveAll(this);
        FEditorDelegates::EndPIE.RemoveAll(this);
    #endif
    FCoreDelegates::OnPreExit.RemoveAll(this);

    if (bEnableAutoCleanup)
    {
        CleanupStaticHandles();
        CleanupInstanceHandles();
    }

    if (Instance == this)
    {
        Instance->RemoveFromRoot();
        Instance = nullptr;
    }
}