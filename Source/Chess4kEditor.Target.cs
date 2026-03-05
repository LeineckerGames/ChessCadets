// Fill out your copyright notice in the Description page of Project Settings.

using System;
using System.Collections.Generic;
using UnrealBuildTool;

public class Chess4kEditorTarget : TargetRules
{
	public Chess4kEditorTarget(TargetInfo Target) : base(Target)
	{

        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V6;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
        ExtraModuleNames.AddRange(new string[] { "Chess4k" });
         
    }
}
