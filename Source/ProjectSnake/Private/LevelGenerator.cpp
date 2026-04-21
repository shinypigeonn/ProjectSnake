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
	
	if (!WallClassA || !WallClassB) return;
	
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
				AWallActor* Wall = GetWorld()->SpawnActor<AWallActor>(
					TileClass, Location, FRotator::ZeroRotator, Params);

				 if (Wall)
				 {
				 	float TileScale = CellSize / 100.0f;
					Wall->SetActorScale3D(FVector(TileScale, TileScale, 1.0f));
				 }
			}
			else if (FloorClassA && FloorClassB)
			{
				// Spawn floor tile so grid is visible
			    TSubclassOf<AFloorActor> TileClass = bIsEven ? FloorClassA : FloorClassB;
			    AFloorActor* Floor = GetWorld()->SpawnActor<AFloorActor>(
			        TileClass, Location, FRotator::ZeroRotator, Params);

			    if (Floor)
			    {
				    // Plane mesh is 200x200 at scale 1.0
			    	// So scale = CellSize / 200
			    	float TileScale = CellSize / 100.0f;
			    	Floor->SetActorScale3D(FVector(TileScale, TileScale, 0.2f));
			    }
			}
		}
	}
}
