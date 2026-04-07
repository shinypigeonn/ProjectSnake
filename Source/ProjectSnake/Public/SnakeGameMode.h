// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SnakeGameMode.generated.h"

UENUM(BlueprintType)
enum class EGameState : uint8
{
	MainMenu	     UMETA(DisplayName = "Main Menu"),
	WaitingToStart   UMETA(DisplayName = "Wait to Start Game"), 
	Playing		     UMETA(DisplayName = "Playing"),
	GameOver	     UMETA(DisplayName = "Game Over"),
	Restarting       UMETA(DisplayName = "Restart"),
};

UCLASS()
class PROJECTSNAKE_API ASnakeGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
	public:
	ASnakeGameMode();
	UFUNCTION(BlueprintCallable, Category="GameMode")
	void MainMenu();
	
	UFUNCTION(BlueprintCallable, Category="GameMode")
	void OnPlay();	
	
	UFUNCTION(BlueprintCallable, Category="GameMode")
	void OnGameOver();
	
	UFUNCTION(BlueprintCallable, Category="GameMode")
	void RestartGame();
	
	UFUNCTION(BlueprintPure, Category="GameMode")
	EGameState GetCurrentState() const { return CurrentState; }
	
	protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, Category="GameMode")
	float RestartDelay = 3.0f;
	
	private:
	EGameState CurrentState = EGameState::MainMenu;
	
	FTimerHandle RestartTimer;
	
	void SetGameState(EGameState newState);
};
