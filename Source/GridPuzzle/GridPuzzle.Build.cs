using UnrealBuildTool;

public class GridPuzzle : ModuleRules
{
	public GridPuzzle(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        
		PublicIncludePaths.AddRange(new string[] { 
			"GridPuzzle/Public" 
		});
        
		PrivateIncludePaths.AddRange(new string[] { 
			"GridPuzzle/Private" 
		});

		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore",
			"Slate",
			"SlateCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });
	}
}