// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class HBTarget : TargetRules
{
	public HBTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;

		bUseLoggingInShipping = true;
        bValidateFormatStrings = true;

        ExtraModuleNames.Add("ModuleNetworkEOS");
        ExtraModuleNames.Add("HB");
    }
}
