using UnrealBuildTool;
using System.IO;

public class HoneycombServer : ModuleRules
{
    private string AbseilPath { get; }

    public HoneycombServer(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Cpp23;
        bEnableExceptions = true;
        bUseUnity = false;

        AbseilPath = Path.Combine(EngineDirectory, "Source", "ThirdParty", "abseil", "20240722.0");

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "Projects",
            "Json",
            "JsonUtilities",
            "Sockets",
            "Networking",
            "HTTPServer"
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
            "CoreUObject",
            "Engine",
            "Slate",
            "SlateCore"
        });

        PublicIncludePaths.Add(Path.Combine(AbseilPath, "include"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public", "Network"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public", "Game"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public", "Database"));
        PublicIncludePaths.Add(Path.Combine(EngineDirectory, "Source", "Runtime", "Online", "HTTPServer", "Public"));

        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "Network"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "Game"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "Database"));

        string AbseilLibPath = Path.Combine(AbseilPath, "lib", "Win64", "x64", "Release");
        if (Target.Platform == UnrealTargetPlatform.Win64 && Directory.Exists(AbseilLibPath))
        {
            foreach (string AbseilLib in Directory.GetFiles(AbseilLibPath, "*.lib"))
            {
                PublicAdditionalLibraries.Add(AbseilLib);
            }
        }
    }
}