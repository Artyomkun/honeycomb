using UnrealBuildTool;

public class Honeycomb : ModuleRules
{
	public Honeycomb(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(new string[]
		{
			ModuleDirectory,
			ModuleDirectory + "/Public",
			ModuleDirectory + "/Public/Systems",
			ModuleDirectory + "/Public/UI",
		});
		PrivateIncludePaths.AddRange(new string[]
		{
			ModuleDirectory + "/Private",
			ModuleDirectory + "/Private/Systems",
			ModuleDirectory + "/Private/UI",
		});

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"UMG",
			"Slate",
			"SlateCore",
			"Json",
			"JsonUtilities",
			"HTTP",
			"HeadMountedDisplay",
			"OpenXR",
			"NavigationSystem",
			"AIModule",
			"GameplayTasks",
			"PhysicsCore",
			"ProceduralMeshComponent",
			"HoneycombCore",
		});
	}
}
