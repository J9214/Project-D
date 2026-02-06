// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/MassAI/CollisionProxyActor.h"
#include "Components/CapsuleComponent.h"
#include "AI/MassAI/MassProxyPoolSubsystem.h"

ACollisionProxyActor::ACollisionProxyActor()
	:CapsuleRadius(34.0f),
	CapsuleHalfHeight(88.0f)
{
	// only Server
	bReplicates = false;
	SetActorHiddenInGame(true);

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("ProxyCapsule"));
	SetRootComponent(Capsule);

	Capsule->InitCapsuleSize(CapsuleRadius, CapsuleHalfHeight);

	Capsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Capsule->SetCollisionResponseToAllChannels(ECR_Ignore);

	// Player Bullet Channel
	Capsule->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);

	ResetProxy();
}

void ACollisionProxyActor::SetRepresentedEntity(const FMassEntityHandle& InEntity)
{
	RepresentedEntity = InEntity;

	if (IsValid(Capsule) == true)
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}

void ACollisionProxyActor::ResetProxy()
{
	RepresentedEntity = FMassEntityHandle();

	if (IsValid(Capsule) == true)
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}