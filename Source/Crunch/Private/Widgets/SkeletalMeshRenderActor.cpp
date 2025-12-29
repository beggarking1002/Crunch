// Fill out your copyright notice in the Description page of Project Settings.


#include "SkeletalMeshRenderActor.h"


// Sets default values
ASkeletalMeshRenderActor::ASkeletalMeshRenderActor()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh Comp");
	MeshComp->SetupAttachment(GetRootComponent());
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetLightingChannels(false, true, false);
}

void ASkeletalMeshRenderActor::ConfigureSkeletalMesh(USkeletalMesh* MeshAsset, TSubclassOf<UAnimInstance> AnimBlueprint)
{
	MeshComp->SetSkeletalMeshAsset(MeshAsset);
	MeshComp->SetAnimInstanceClass(AnimBlueprint);
}

// Called when the game starts or when spawned
void ASkeletalMeshRenderActor::BeginPlay()
{
	Super::BeginPlay();
	MeshComp->SetVisibleInSceneCaptureOnly(true);
}

