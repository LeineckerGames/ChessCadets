// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class Chess4k : ModuleRules
{
	public Chess4k(ReadOnlyTargetRules Target) : base(Target)
	{
		//PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PCHUsage = PCHUsageMode.NoPCHs;

        bEnableExceptions = true;
        bUseUnity = false;
        bUseRTTI = true;
        CppStandard = CppStandardVersion.Cpp20;

		PublicIncludePaths.Add("$(ModuleDir)/PulseChess");
		PublicSystemLibraries.Add("msvcprt.lib");

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
