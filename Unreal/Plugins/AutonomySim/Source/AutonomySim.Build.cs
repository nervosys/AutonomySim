// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

using UnrealBuildTool;
using System.IO;

public class AutonomySim : ModuleRules
{
    private string ModulePath
    {
        get { return ModuleDirectory; }
    }

    private string AutonomyLibPath
    {
        get { return Path.Combine(ModulePath, "AutonomyLib"); }
    }
    private string AutonomySimPluginPath
    {
        get { return Directory.GetParent(ModulePath).FullName; }
    }
    private string ProjectBinariesPath
    {
        get { return Path.Combine(
                Directory.GetParent(AutonomySimPluginPath).Parent.FullName, "Binaries");
        }
    }
    private string AutonomySimPluginDependencyPath
    {
        get { return Path.Combine(AutonomySimPluginPath, "Dependencies"); }
    }

    private enum CompileMode
    {
        HeaderOnlyNoRpc,
        HeaderOnlyWithRpc,
        CppCompileNoRpc,
        CppCompileWithRpc
    }

    private void SetupCompileMode(CompileMode mode, ReadOnlyTargetRules Target)
    {
        LoadAutonomySimDependency(Target, "MavLinkCom", "MavLinkCom");

        switch (mode)
        {
            case CompileMode.HeaderOnlyNoRpc:
                PublicDefinitions.Add("AIRLIB_HEADER_ONLY=1");
                PublicDefinitions.Add("AIRLIB_NO_RPC=1");
                AddLibDependency("AutonomyLib", Path.Combine(AutonomyLibPath, "lib"), "AutonomyLib", Target, false);
                break;

            case CompileMode.HeaderOnlyWithRpc:
                PublicDefinitions.Add("AIRLIB_HEADER_ONLY=1");
                AddLibDependency("AutonomyLib", Path.Combine(AutonomyLibPath, "lib"), "AutonomyLib", Target, false);
                LoadAutonomySimDependency(Target, "rpclib", "rpc");
                break;

            case CompileMode.CppCompileNoRpc:
                LoadAutonomySimDependency(Target, "MavLinkCom", "MavLinkCom");
                PublicDefinitions.Add("AIRLIB_NO_RPC=1");
                break;

            case CompileMode.CppCompileWithRpc:
                LoadAutonomySimDependency(Target, "rpclib", "rpc");
                break;

            default:
                throw new System.Exception("CompileMode specified in plugin's Build.cs file is not recognized");
        }

    }

    public AutonomySim(ReadOnlyTargetRules Target) : base(Target)
    {
        //bEnforceIWYU = true; //to support 4.16
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        bEnableExceptions = true;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "ImageWrapper", "RenderCore", "RHI", "PhysicsCore", "AssetRegistry", "ChaosVehicles", "Landscape", "CinematicCamera" });
        PrivateDependencyModuleNames.AddRange(new string[] { "UMG", "Slate", "SlateCore", "RenderCore" });

        //suppress VC++ proprietary warnings
        PublicDefinitions.Add("_SCL_SECURE_NO_WARNINGS=1");
        PublicDefinitions.Add("_CRT_SECURE_NO_WARNINGS=1");
        PublicDefinitions.Add("HMD_MODULE_INCLUDED=0");

        PublicIncludePaths.Add(Path.Combine(AutonomyLibPath, "include"));
        PublicIncludePaths.Add(Path.Combine(AutonomyLibPath, "deps", "eigen3"));
        AddOSLibDependencies(Target);

        SetupCompileMode(CompileMode.HeaderOnlyWithRpc, Target);
    }

    private void AddOSLibDependencies(ReadOnlyTargetRules Target)
    {
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            // for SHGetFolderPath.
            PublicAdditionalLibraries.Add("Shell32.lib");

            //for joystick support
            PublicAdditionalLibraries.Add("dinput8.lib");
            PublicAdditionalLibraries.Add("dxguid.lib");
        }

		if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			// needed when packaging
			PublicAdditionalLibraries.Add("stdc++");
			PublicAdditionalLibraries.Add("supc++");
		}
    }

    static void CopyFileIfNewer(string srcFilePath, string destFolder)
    {
        FileInfo srcFile = new FileInfo(srcFilePath);
        FileInfo destFile = new FileInfo(Path.Combine(destFolder, srcFile.Name));
        if (!destFile.Exists || srcFile.LastWriteTime > destFile.LastWriteTime)
        {
            srcFile.CopyTo(destFile.FullName, true);
        }
        //else skip
    }

    private bool LoadAutonomySimDependency(ReadOnlyTargetRules Target, string LibName, string LibFileName)
    {
        string LibrariesPath = Path.Combine(AutonomyLibPath, "deps", LibName, "lib");
        return AddLibDependency(LibName, LibrariesPath, LibFileName, Target, true);
    }

    private bool AddLibDependency(string LibName, string LibPath, string LibFileName, ReadOnlyTargetRules Target, bool IsAddLibInclude)
    {
        string PlatformString = (Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.Mac) ? "x64" : "x86";
        string ConfigurationString = (Target.Configuration == UnrealTargetConfiguration.Debug) ? "Debug" : "Release";
        bool isLibrarySupported = false;


        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            isLibrarySupported = true;

            PublicAdditionalLibraries.Add(Path.Combine(LibPath, PlatformString, ConfigurationString, LibFileName + ".lib"));
        } else if (Target.Platform == UnrealTargetPlatform.Linux || Target.Platform == UnrealTargetPlatform.Mac) {
            isLibrarySupported = true;
            PublicAdditionalLibraries.Add(Path.Combine(LibPath, "lib" + LibFileName + ".a"));
        }

        if (isLibrarySupported && IsAddLibInclude)
        {
            // Include path
            PublicIncludePaths.Add(Path.Combine(AutonomyLibPath, "deps", LibName, "include"));
        }
        PublicDefinitions.Add(string.Format("WITH_" + LibName.ToUpper() + "_BINDING={0}", isLibrarySupported ? 1 : 0));

        return isLibrarySupported;
    }
}
