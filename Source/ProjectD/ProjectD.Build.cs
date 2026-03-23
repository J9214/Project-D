// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class ProjectD : ModuleRules
{
	public ProjectD(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDefinitions.Add("UE_PROJECT_STEAMSHIPPINGID=0");

        PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
            "EnhancedInput",
            "AIModule",
            "UMG",
            "Slate",
            "SlateCore",
            "Niagara",

			// GAS
			"GameplayAbilities",
            "GameplayTags",
            "GameplayTasks",

			// NetWork
            "Sockets",
            "Networking",
            "NetCore",
            "OnlineSubsystem",
            "OnlineSubsystemUtils",
            "OnlineSubsystemSteam",
            "SteamSockets",
			"CoreOnline",    

			// Navigation
			"NavigationSystem",

			// State Tree
			"StateTreeModule",
            "GameplayStateTreeModule",

			// Mover
			"Mover",

            // Mass AI
            "MassEntity",
            "MassCommon",
            "MassMovement",
            "MassSpawner",
            "MassRepresentation",
            "MassSignals",
            "StructUtils",
            "MassNavigation",
            "MassAIBehavior",
            "MassReplication",
            "MassLOD",
            "MassSimulation",

            //Niagara
            "Niagara",

            // Geometry Collection
            "GeometryCollectionEngine",

            // Cable
            "CableComponent",

            //UI
            "CommonUI",

            "AnimationDataController"
        });


        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
