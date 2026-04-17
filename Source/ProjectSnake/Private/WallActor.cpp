// Fill out your copyright notice in the Description page of Project Settings.


#include "WallActor.h"

// Sets default values
AWallActor::AWallActor()
{
	WallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WallMesh"));
	RootComponent = WallMesh;
	
	WallMesh->SetCollisionProfileName(TEXT("BlockAll"));
	
	// Tagging it via code so OnHit can identify it (before I tagged it in the blueprint)
	Tags.Add(TEXT("Wall"));
}


