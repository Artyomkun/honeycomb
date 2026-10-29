#pragma once

#include "HoneycombCore.h"
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class DummyEditorScript: public IModuleInterface
{
    public:
        virtual void StartupModule() override;
        virtual void ShutdownModule() override;

    private:
        void OnEditorStartup();
        void OnEditorShutdown();
        void BindEditorDelegates();
};