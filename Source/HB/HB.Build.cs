// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HB : ModuleRules
{
	public HB(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] 
		{	
            "Core",
            "CoreUObject",
			"Engine",
			"AIModule",
			"NavigationSystem",
			"GameplayTasks",
			"Niagara",
			"ModuleNetworkEOS",
        });

		PrivateDependencyModuleNames.AddRange(new string[]
		{
            "InputCore",
            "EnhancedInput",
            "Slate",
			"SlateCore",
			"UMG",
			"ModuleNetworkEOS"
		});
    }
}
