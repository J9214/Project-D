// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/MassAI/CollisionProxyActor.h"
#include "Components/CapsuleComponent.h"

ACollisionProxyActor::ACollisionProxyActor()
{
	// only Server
	bReplicates = false;
	SetActorHiddenInGame(true);

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("ProxyCapsule"));
	SetRootComponent(Capsule);

	Capsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Capsule->SetCollisionResponseToAllChannels(ECR_Ignore);

	// Player Bullet Channel
	Capsule->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);

	ResetProxy();
}

void ACollisionProxyActor::SetRepresentedEntity(const FMassEntityHandle& InEntity)
{
	RepresentedEntity = InEntity;
	RepresentedEntityKey = InEntity.IsValid() ? MakeEntityKey(InEntity) : 0;
}

void ACollisionProxyActor::ResetProxy()
{
	RepresentedEntity = FMassEntityHandle();
	RepresentedEntityKey = 0;

	if (Capsule)
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

int64 ACollisionProxyActor::MakeEntityKey(const FMassEntityHandle& E)
{
	return (static_cast<int64>(E.Index) << 32) | static_cast<int64>(E.SerialNumber);
}