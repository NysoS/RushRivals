// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class HBEditorTarget : TargetRules
{
	public HBEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;

        bValidateFormatStrings = true;

        ExtraModuleNames.Add("ModuleNetworkEOS");
        ExtraModuleNames.Add("HB");
    }
}
