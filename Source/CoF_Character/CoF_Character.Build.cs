// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CoF_Character : ModuleRules
{
	public CoF_Character(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"CoF_Character",
			"CoF_Character/Variant_Platforming",
			"CoF_Character/Variant_Platforming/Animation",
			"CoF_Character/Variant_Combat",
			"CoF_Character/Variant_Combat/AI",
			"CoF_Character/Variant_Combat/Animation",
			"CoF_Character/Variant_Combat/Gameplay",
			"CoF_Character/Variant_Combat/Interfaces",
			"CoF_Character/Variant_Combat/UI",
			"CoF_Character/Variant_SideScrolling",
			"CoF_Character/Variant_SideScrolling/AI",
			"CoF_Character/Variant_SideScrolling/Gameplay",
			"CoF_Character/Variant_SideScrolling/Interfaces",
			"CoF_Character/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
