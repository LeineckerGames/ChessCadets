using UnrealBuildTool;
using System.IO;

public class Chess4k : ModuleRules
{
    public Chess4k(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.NoPCHs;
        bEnableExceptions = true;
        bUseUnity = false;
        bUseRTTI = true;
        CppStandard = CppStandardVersion.Cpp20;

        PublicIncludePaths.Add("$(ModuleDir)/PulseChess");

        // Explicitly add PulseChess source directory
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "PulseChess"));

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });
        PrivateDependencyModuleNames.AddRange(new string[] { });
    }
}