// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TestZone : ModuleRules
{
  public TestZone(ReadOnlyTargetRules Target) : base(Target)
  {
    PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

    PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "AIModule",
            "NavigationSystem",
            "StateTreeModule",
            "GameplayStateTreeModule",
            "Niagara",
            "UMG",
            "Slate",
            "MetasoundEngine",
            "MetasoundFrontend",
            "AudioMixer",
            "Sockets"
        });

    PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

    PublicIncludePaths.AddRange(new string[] {
            "TestZone",
            "TestZone/Test_CodeSet",
            "TestZone/Network",
            "TestZone/Gameplay"
        });

    // Uncomment if you are using Slate UI
    // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

    // Uncomment if you are using online features
    // PrivateDependencyModuleNames.Add("OnlineSubsystem");

    // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
  }
}
