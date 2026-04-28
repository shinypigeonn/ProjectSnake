// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FloorActor.h"
#include "Food.h"
#include "GameFramework/Actor.h"
#include "LevelGenerator.generated.h"

class AWallActor;

UCLASS()
class PROJECTSNAKE_API ALevelGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	ALevelGenerator();
	FVector GridToWorld(FIntPoint GridPos) const;

protected:
	virtual void BeginPlay() override;
	
	// Wall tiles in grid 
	UPROPERTY(EditDefaultsOnly, Category="Level")
	TSubclassOf<AWallActor> WallClassA;
	
	UPROPERTY(EditDefaultsOnly, Category="Level")
	TSubclassOf<AWallActor> WallClassB;
	
	// Floor tiles so the grid is visible
	UPROPERTY(EditDefaultsOnly, Category="Level")
	TSubclassOf<AFloorActor> FloorClassA;
	
	UPROPERTY(EditDefaultsOnly, Category="Level")
	TSubclassOf<AFloorActor> FloorClassB;
	
	// Grid properties
	UPROPERTY(EditDefaultsOnly, Category="Level")
	int32 GridWidth = 50;
	
	UPROPERTY(EditDefaultsOnly, Category="Level")
	int32 GridHeight = 50;
	
	UPROPERTY(EditDefaultsOnly, Category="Level")
	float CellSize = 100.0f;

	// Interior wall generation
	UPROPERTY(EditDefaultsOnly, Category="Level|Interior Walls")
	int32 InteriorWallCount = 10;
	
	UPROPERTY(EditDefaultsOnly, Category="Level|Interior Walls")
	int32 MaxWallSegmentLength = 4;
	
	// Food classes
	UPROPERTY(EditDefaultsOnly, Category="Level|Food")
	TSubclassOf<AFood> BasicFoodClass;
	
	UPROPERTY(EditDefaultsOnly, Category="Level|Food")
	TSubclassOf<AFood> SpecialFoodClass;
	
	UPROPERTY(EditDefaultsOnly, Category="Level|Food")
	TSubclassOf<AFood> SpeedBoostFoodClass;
	
	UPROPERTY(EditDefaultsOnly, Category="Level|Food")
	TSubclassOf<AFood> GoldenFoodClass;

	// How many food actors are alive at once
	UPROPERTY(EditDefaultsOnly, Category="Level|Food")
	int32 BasicFoodCount = 3;
	
	UPROPERTY(EditDefaultsOnly, Category="Level|Food")
	float SpecialFoodInterval = 20.0f;
	
	UPROPERTY(EditDefaultsOnly, Category="Level|Food")
	float SpeedBoostFoodInterval = 15.0f;
	
	UPROPERTY(EditDefaultsOnly, Category="Level|Food")
	float GoldenFoodInterval = 60.0f;

private: 
	TSet<FIntPoint> WallCells;
	
	// Currently live food actors (so we can check counts)
	TArray<TWeakObjectPtr<AFood>> ActiveBasicFood;
	
	FTimerHandle BasicFoodTimer;
	FTimerHandle SpecialFoodTimer;
	FTimerHandle SpeedBoostFoodTimer;
	FTimerHandle GoldenFoodTimer;
 
	void GenerateWalls() const;
	void GenerateInteriorWalls();
	void SpawnWallAt(FIntPoint GridPos);
 
	void StartFoodTimers();
	void TrySpawnBasicFood();
	void SpawnSpecialFood();
	void SpawnSpeedBoostFood();
	void SpawnGoldenFood();
	void SpawnFoodOfClass(TSubclassOf<AFood> FoodClass);
 
	FIntPoint GetRandomEmptyCell() const;
 
	// Returns the spawned AActor* so TrySpawnBasicFood can track it
	AActor* SpawnFoodOfClass_Internal(TSubclassOf<AFood> FoodClass);
};
