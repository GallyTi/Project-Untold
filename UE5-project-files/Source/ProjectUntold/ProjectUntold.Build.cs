using UnrealBuildTool;

public class ProjectUntold : ModuleRules
{
	public ProjectUntold(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"HTTP",
			"Json",
			"JsonUtilities",
			"OnlineSubsystemEOS",
			"OnlineSubsystemEOSPlus",
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
			"VoiceChat",
			"UMG",
			"Slate",
			"SlateCore",
			"Paper2D",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"ImageDownload",
			"RigLogicLib"
		});

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"CurveEditor",
				"WidgetAutomationTests"
			});
		}
	}
}