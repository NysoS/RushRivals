// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class HBServerTarget : TargetRules
{
	public HBServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;

		bUseLoggingInShipping = true;
        bValidateFormatStrings = true;

        ExtraModuleNames.Add("HB");
		ExtraModuleNames.Add("ModuleNetworkEOS");
	}
}
