// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelGenerator.h"
#include "WallActor.h"

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

void ALevelGenerator::GenerateWalls() const
{
	if (!WallClassA || WallClassB) return;
	
	for (int32 X = 0; X < GridWidth; X++)
	{
		for (int32 Y = 0; Y < GridHeight; Y++)
		{
			bool const bIsBorder = X == 0 || X == GridWidth - 1 
			|| Y == 0 || Y == GridHeight - 1;
			
			bool const bIsEven = (X + Y) % 2 == 0;
			
			FVector Location = GridToWorld(FIntPoint(X, Y));
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride =
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
         		
			if (bIsBorder)
			{
				// Spawn wall tile
				TSubclassOf<AWallActor> TileClass = bIsEven ? WallClassA : WallClassB;
				GetWorld()->SpawnActor<AWallActor>(
					TileClass, Location, FRotator::ZeroRotator, Params);
			}
			else if (FloorClassA && FloorClassB)
			{
				// Spawn floor tile so grid is visible
				TSubclassOf<AWallActor> TileClass = bIsEven ? FloorClassA : FloorClassB;
				GetWorld()->SpawnActor<AWallActor>(
					TileClass, Location, FRotator::ZeroRotator, Params);
			}
		}
	}
}
