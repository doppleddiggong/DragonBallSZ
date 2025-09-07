// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

using UnrealBuildTool;

public class CoffeeToolbar : ModuleRules
{
	public CoffeeToolbar(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core", "CoreUObject", "Engine"
			});
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",

				"Slate",
				"SlateCore",

				"ToolMenus",
				"UnrealEd",
				"LevelEditor",

				"AssetRegistry",
				"DeveloperSettings",
			});
	}
}
