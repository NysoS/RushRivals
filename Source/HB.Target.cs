// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class HBTarget : TargetRules
{
	public HBTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_2;
		bUseLoggingInShipping = true;
		ExtraModuleNames.Add("HB");
	    ExtraModuleNames.Add("ModuleNetworkEOS");
		ExtraModuleNames.Add("CPathfinding");
	}
}