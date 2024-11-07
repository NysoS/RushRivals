using UnrealBuildTool;

public class ModuleNetworkEOS: ModuleRules
{
    public ModuleNetworkEOS(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(new string[] {"Core", "CoreUObject", "Engine", "DeveloperSettings", "UMG" });
        PublicDependencyModuleNames.AddRange(new string[] { "OnlineSubsystemEOS", "OnlineSubsystem", "OnlineSubsystemUtils", "MoviePlayer" });

        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string [] {
                "UnrealEd",
            });
        }
    }
}