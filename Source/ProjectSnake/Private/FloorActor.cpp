// Fill out your copyright notice in the Description page of Project Settings.


#include "FloorActor.h"

// Sets default values
AFloorActor::AFloorActor()
{
	FloorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FloorMesh"));
	RootComponent = FloorMesh;
	
	FloorMesh->SetCollisionProfileName(TEXT("NoCollision"));
	
}

