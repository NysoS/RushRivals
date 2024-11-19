// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class HBServerTarget : TargetRules
{
	public HBServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_2;
		bUseLoggingInShipping = true;
        ExtraModuleNames.Add("HB");
		ExtraModuleNames.Add("ModuleNetworkEOS");
	}
}
