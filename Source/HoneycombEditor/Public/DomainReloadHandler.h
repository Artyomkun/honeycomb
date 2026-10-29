#pragma once

#include "HoneycombCore.h"
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DomainReloadHandler.generated.h"

struct HandleInfo
{
    FString Name;
    void* Pointer = nullptr;
    FDateTime AllocationTime;
    FString AllocationStack;
    bool bIsAllocated = false;
};

DECLARE_DELEGATE(FOnDomainReload);
DECLARE_MULTICAST_DELEGATE(FOnCleanupComplete);

UCLASS(Blueprintable, BlueprintType)
class HONEYCOMBEDITOR_API UDomainReloadHandler : public UObject
{
    GENERATED_BODY()

    public:
        UDomainReloadHandler();
        static UDomainReloadHandler* Get();
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domain Reload")
        bool bEnableAutoCleanup = true;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domain Reload")
        bool bLogCleanupOperations = true;

        void RegisterStaticHandle(void* Pointer, const FString& Name);
        void UnregisterStaticHandle(void* Pointer);
        void RegisterInstanceHandle(void* Pointer, const FString& Name);
        void UnregisterInstanceHandle(void* Pointer);

        UFUNCTION(BlueprintCallable, Category = "Domain Reload")
        void CleanupStaticHandles();

        UFUNCTION(BlueprintCallable, Category = "Domain Reload")
        void CleanupInstanceHandles();

        UFUNCTION(BlueprintCallable, Category = "Domain Reload")
        void ForceCleanup();

        UFUNCTION(BlueprintCallable, Category = "Domain Reload")
        void LogActiveHandles();

        UFUNCTION(BlueprintCallable, Category = "Domain Reload")
        int32 GetStaticHandleCount() const;

        UFUNCTION(BlueprintCallable, Category = "Domain Reload")
        int32 GetInstanceHandleCount() const;

        FOnCleanupComplete OnStaticCleanupComplete;
        FOnCleanupComplete OnInstanceCleanupComplete;

    protected:
        void InitializeHandler();
        void CleanupHandles(TArray<HandleInfo>& Handles, const FString& HandleType);

        #if WITH_EDITOR
            void OnPreBeginPIE(bool bIsSimulating);
            void OnEndPIE(bool bIsSimulating);
        #endif

        void OnPreExit();

        TArray<HandleInfo> StaticHandles;
        TArray<HandleInfo> InstanceHandles;

        static UDomainReloadHandler* Instance;

    private:
        void LogCleanup(const FString& Message);
};