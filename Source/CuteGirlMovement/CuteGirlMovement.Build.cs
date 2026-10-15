// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CuteGirlMovement : ModuleRules
{
	public CuteGirlMovement(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"CuteGirlMovement",
			"CuteGirlMovement/Variant_Platforming",
			"CuteGirlMovement/Variant_Platforming/Animation",
			"CuteGirlMovement/Variant_Combat",
			"CuteGirlMovement/Variant_Combat/AI",
			"CuteGirlMovement/Variant_Combat/Animation",
			"CuteGirlMovement/Variant_Combat/Gameplay",
			"CuteGirlMovement/Variant_Combat/Interfaces",
			"CuteGirlMovement/Variant_Combat/UI",
			"CuteGirlMovement/Variant_SideScrolling",
			"CuteGirlMovement/Variant_SideScrolling/AI",
			"CuteGirlMovement/Variant_SideScrolling/Gameplay",
			"CuteGirlMovement/Variant_SideScrolling/Interfaces",
			"CuteGirlMovement/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
