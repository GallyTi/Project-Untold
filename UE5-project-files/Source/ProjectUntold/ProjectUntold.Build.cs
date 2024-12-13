// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProjectUntold : ModuleRules
{
	public ProjectUntold(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(new string[] { "ImageDownload", "CurveEditor", "VoiceChat", "VoiceChat", "WidgetAutomationTests", "WidgetAutomationTests", "RigLogicLib" });
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput",
			"HTTP", "Json", "JsonUtilities", "OnlineSubsystemEOS", "OnlineSubsystemEOSPlus" ,
			"OnlineSubsystem", "OnlineSubsystemUtils"
		});
	}
}
