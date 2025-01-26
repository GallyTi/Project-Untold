using UnrealBuildTool;
using System.Collections.Generic;
using System;

public class ProjectUntoldClientTarget : TargetRules
{
	public ProjectUntoldClientTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Client;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
		ExtraModuleNames.Add("ProjectUntold");
	}
}