using UnrealBuildTool;
using System.IO;

public class ChessKids : ModuleRules
{
	public ChessKids(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.NoPCHs;
		bEnableExceptions = true;
		bUseUnity = false;
		bUseRTTI = true;
		CppStandard = CppStandardVersion.Cpp20;

		// Pulse chess engine — embedded source
		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Engine"));

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });
		PrivateDependencyModuleNames.AddRange(new string[] { });
	}
}
