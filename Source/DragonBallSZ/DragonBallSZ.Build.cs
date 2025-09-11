// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class DragonBallSZ : ModuleRules
{
	public DragonBallSZ(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",

			"Niagara",
			
			"AIModule",				
			"GameplayTasks",		
			"NavigationSystem",		
			
			"CoffeeLibrary"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		PublicIncludePaths.AddRange(new string[] {
			Path.Combine(ModuleDirectory, "Common", "Public"),
			Path.Combine(ModuleDirectory, "Character", "Public"),
			Path.Combine(ModuleDirectory, "Enviroment", "Public"),
			Path.Combine(ModuleDirectory, "MasterData", "Public"),
			Path.Combine(ModuleDirectory, "UI", "Public"),
		});
	}
}
