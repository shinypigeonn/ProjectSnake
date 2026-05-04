// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SnakeGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTSNAKE_API USnakeGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite, Category="Game")
	bool bIsMultiplayer = false;
	
	UPROPERTY(BlueprintReadWrite, Category="Score")
	int32 FinalScore = 0;
	
	UPROPERTY(BlueprintReadWrite, Category="Score")
	int32 FinalScoreP2 = 0;
};
