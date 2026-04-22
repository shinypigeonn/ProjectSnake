// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelGenerator.h"
#include "Food.h"
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
	GenerateInteriorWalls();
	StartFoodTimers();
}

// ────────────────────────────────────────────────────────────────────────────────────────────────────────
// GRID HELPERS
// ────────────────────────────────────────────────────────────────────────────────────────────────────────
#pragma region GRID HELPERS
FVector ALevelGenerator::GridToWorld(FIntPoint GridPos) const
{
	return FVector(
		GridPos.X * CellSize + CellSize * 0.5f,
		GridPos.Y * CellSize + CellSize * 0.5f,
		GetActorLocation().Z
	);
}
 
FIntPoint ALevelGenerator::GetRandomEmptyCell() const
{
	// Collect all inner cells not occupied by a wall
	TArray<FIntPoint> Candidates;
	for (int32 X = 1; X < GridWidth - 1; X++)
	{
		for (int32 Y = 1; Y < GridHeight - 1; Y++)
		{
			FIntPoint Pos(X, Y);
			if (!WallCells.Contains(Pos))
				Candidates.Add(Pos);
		}
	}
 
	if (Candidates.IsEmpty())
		return FIntPoint(GridWidth / 2, GridHeight / 2); // fallback centre
 
	return Candidates[FMath::RandRange(0, Candidates.Num() - 1)];
}
#pragma endregion
// ────────────────────────────────────────────────────────────────────────────────────────────────────────
// WALL GENERATION
// ────────────────────────────────────────────────────────────────────────────────────────────────────────
#pragma region WALL GENERATION | Generating walls, floor, inside walls
void ALevelGenerator::SpawnWallAt(FIntPoint GridPos)
{
	if (!WallClassA || !WallClassB) return;
 
	TSubclassOf<AWallActor> TileClass = ((GridPos.X + GridPos.Y) % 2 == 0) ? WallClassA : WallClassB;
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
 
	AWallActor* Wall = GetWorld()->SpawnActor<AWallActor>(TileClass, GridToWorld(GridPos), FRotator::ZeroRotator, Params);
	if (Wall)
	{
		float TileScale = CellSize / 100.0f;
		Wall->SetActorScale3D(FVector(TileScale, TileScale, 1.0f));
	}
	WallCells.Add(GridPos);
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
			    	float TileScale = CellSize / 100.0f;
			    	Floor->SetActorScale3D(FVector(TileScale, TileScale, 0.2f));
			    }
			}
		}
	}
}

void ALevelGenerator::GenerateInteriorWalls()
{
	if (!WallClassA || !WallClassB) return;
 
	FIntPoint Centre(GridWidth / 2, GridHeight / 2);
	const int32 SafeRadius = 3;
	const FIntPoint Dirs[4] = { {1,0}, {0,1}, {-1,0}, {0,-1} };
 
	for (int32 i = 0; i < InteriorWallCount; i++)
	{
		FIntPoint Start;
		int32 Attempts = 0;
		do
		{
			Start = FIntPoint(FMath::RandRange(2, GridWidth - 3), FMath::RandRange(2, GridHeight - 3));
			Attempts++;
		}
		while ((FMath::Abs(Start.X - Centre.X) <= SafeRadius && FMath::Abs(Start.Y - Centre.Y) <= SafeRadius)
			|| WallCells.Contains(Start) || Attempts > 50);
 
		if (Attempts > 50) continue;
 
		FIntPoint Dir = Dirs[FMath::RandRange(0, 3)];
		int32 Length = FMath::RandRange(2, MaxWallSegmentLength);
		FIntPoint Current = Start;
 
		for (int32 Step = 0; Step < Length; Step++)
		{
			if (Current.X <= 0 || Current.X >= GridWidth - 1 || Current.Y <= 0 || Current.Y >= GridHeight - 1) break;
			if (FMath::Abs(Current.X - Centre.X) <= SafeRadius && FMath::Abs(Current.Y - Centre.Y) <= SafeRadius) break;
			if (!WallCells.Contains(Current)) SpawnWallAt(Current);
			Current += Dir;
		}
	}
}
#pragma endregion
// ────────────────────────────────────────────────────────────────────────────────────────────────────────
// FOOD SPAWNING
// ────────────────────────────────────────────────────────────────────────────────────────────────────────
#pragma region FOOD SPAWNING | Food timers & Spawn
void ALevelGenerator::StartFoodTimers()
{
	TrySpawnBasicFood();
	GetWorldTimerManager().SetTimer(
		BasicFoodTimer, 
		this, 
		&ALevelGenerator::TrySpawnBasicFood,
		3.0f, 
		true);
	
	GetWorldTimerManager().SetTimer(
		SpecialFoodTimer, 
		this, 
		&ALevelGenerator::SpawnSpecialFood,
		SpecialFoodInterval,
		true,
		SpecialFoodInterval
	);
	
	GetWorldTimerManager().SetTimer(
		SpeedBoostFoodTimer, 
		this, 
		&ALevelGenerator::SpawnSpeedBoostFood,
		SpeedBoostFoodInterval,
		true,
		SpeedBoostFoodInterval
	);
	
	GetWorldTimerManager().SetTimer(
		GoldenFoodTimer,
		this,
		&ALevelGenerator::SpawnGoldenFood,
		GoldenFoodInterval,
		true);
}

void ALevelGenerator::TrySpawnBasicFood()
{
	ActiveBasicFood.RemoveAll(
		[](const TWeakObjectPtr<AFood>& Ptr) { return !Ptr.IsValid(); });
	
	while (ActiveBasicFood.Num() < BasicFoodCount)
	{
		if (AFood* Spawned = Cast<AFood>(SpawnFoodOfClass_Internal(BasicFoodClass)))
			ActiveBasicFood.Add(Spawned);
		else
			break;
	}
}

void ALevelGenerator::SpawnSpecialFood()    { SpawnFoodOfClass(SpecialFoodClass); }

void ALevelGenerator::SpawnSpeedBoostFood() { SpawnFoodOfClass(SpeedBoostFoodClass); }

void ALevelGenerator::SpawnGoldenFood()     { SpawnFoodOfClass(GoldenFoodClass); }

void ALevelGenerator::SpawnFoodOfClass(TSubclassOf<AFood> FoodClass)
{
	SpawnFoodOfClass_Internal(FoodClass);
}
 
AActor* ALevelGenerator::SpawnFoodOfClass_Internal(TSubclassOf<AFood> FoodClass)
{
	if (!FoodClass || !GetWorld()) return nullptr;
 
	FIntPoint Cell = GetRandomEmptyCell();
	FVector Location = GridToWorld(Cell);
	Location.Z += 50.0f; // lift slightly above the floor
 
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
 
	return GetWorld()->SpawnActor<AActor>(FoodClass, Location, FRotator::ZeroRotator, Params);
}
#pragma endregion