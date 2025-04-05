// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ClimbingSystem : ModuleRules
{
	public ClimbingSystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(new string[] { "MotionWarping" });
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput"
		});
		
		PublicIncludePaths.AddRange(new string[] { "ClimbingSystem/Public" });
	}
}
