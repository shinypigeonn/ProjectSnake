// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelGenerator.h"
#include "WallActor.h"
#include "Components/SplineMeshComponent.h"

// Sets default values
ALevelGenerator::ALevelGenerator()
{
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ALevelGenerator::BeginPlay()
{
	Super::BeginPlay();
	GenerateWalls();
}

FVector ALevelGenerator::GridToWorld(FIntPoint GridPos) const
{
	return FVector(
        GridPos.X * CellSize + CellSize * 0.5f,
        GridPos.Y * CellSize + CellSize * 0.5f,
        GetActorLocation().Z
        );
}

void ALevelGenerator::GenerateWalls()
{
	if (!WallClass) return;
	
	for (int32 X = 0; X < GridWidth; X++)
	{
		for (int32 Y = 0; Y < GridHeight; Y++)
		{
			bool bIsBorder = X == 0 || X == GridWidth - 1 
			|| Y == 0 || Y == GridHeight - 1;
         		
			if (!bIsBorder) continue;
         		
			FVector Location = GridToWorld(FIntPoint(X, Y));
         		
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = 
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
         		
			AWallActor* Wall = GetWorld()->SpawnActor<AWallActor>
			(WallClass, Location, FRotator::ZeroRotator, Params);
			
		}
	}
}
