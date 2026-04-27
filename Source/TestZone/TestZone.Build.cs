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
    "AudioMixer"
        });

    PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

    PublicIncludePaths.AddRange(new string[] {
            "TestZone",
            "TestZone/Variant_Strategy",
            "TestZone/Variant_Strategy/UI",
            "TestZone/Variant_TwinStick",
            "TestZone/Variant_TwinStick/AI",
            "TestZone/Variant_TwinStick/Gameplay",
            "TestZone/Variant_TwinStick/UI"
        });

    // Uncomment if you are using Slate UI
    // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

    // Uncomment if you are using online features
    // PrivateDependencyModuleNames.Add("OnlineSubsystem");

    // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
  }
}
