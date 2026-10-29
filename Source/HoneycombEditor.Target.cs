using UnrealBuildTool;
using System.Collections.Generic;

public class HoneycombEditorTarget : TargetRules
{
	public HoneycombEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;

		ExtraModuleNames.AddRange(new string[] { "Honeycomb", "HoneycombCore", "HoneycombEditor" });
	}
}
