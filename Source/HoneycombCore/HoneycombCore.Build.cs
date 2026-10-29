using UnrealBuildTool;

public class HoneycombCore : ModuleRules
{
	public HoneycombCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.Add(ModuleDirectory);
		PublicIncludePaths.Add(System.IO.Path.Combine(ModuleDirectory, "Public", "Sripts"));
		PrivateIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
		});
	}
}
