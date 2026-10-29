using UnrealBuildTool;

public class HoneycombEditor : ModuleRules
{
	public HoneycombEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"Honeycomb",
			"HoneycombCore",
			"UnrealEd",
			"Blutility",
			"UMG",
			"Slate",
			"SlateCore",
		});
	}
}
