// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FloorActor.h"
#include "GameFramework/Actor.h"
#include "LevelGenerator.generated.h"

class AWallActor;
class ASnakePawn;

UCLASS()
class PROJECTSNAKE_API ALevelGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	ALevelGenerator();

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


public:	
	void GenerateWalls() const;
	FVector GridToWorld(FIntPoint GridPos) const;
};
