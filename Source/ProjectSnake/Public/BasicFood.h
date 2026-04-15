// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Food.h"
#include "BasicFood.generated.h"

/**
 * @brief Represents a basic type of food in the game.
 *
 * ABasicFood is derived from the AFood class and serves as the default implementation
 * for a simple food type consumed by the snake. This class assigns predefined FFoodData
 * specific to the "Basic" food type, which is characterized by a lower point value
 * and no special benefits.
 *
 * The food's behavior and properties can be extended further in blueprints or
 * through additional code.
 */
UCLASS()
class PROJECTSNAKE_API ABasicFood : public AFood
{
	GENERATED_BODY()
	
public:
	ABasicFood();
};
