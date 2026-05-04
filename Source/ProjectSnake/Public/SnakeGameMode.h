// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SnakeGrid.h"
#include "InputMappingContext.h"
#include "SnakeGameMode.generated.h"

class ASnakePawn;

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
	
	FSnakeGrid Grid;
	
	public:
	ASnakeGameMode();
	UFUNCTION(BlueprintCallable, Category="GameMode")
	void MainMenu();
	
	UFUNCTION(BlueprintCallable, Category="GameMode")
	void OnPlay();	
	
	UFUNCTION(BlueprintCallable, Category="GameMode")
	void OnPlayerDied(ASnakePawn* DeadPawn);
	
	UFUNCTION(BlueprintCallable, Category="GameMode")
	void OnGameOver();
	
	UFUNCTION(BlueprintCallable, Category="GameMode")
	void RestartGame();
	
	UFUNCTION(BlueprintPure, Category="GameMode")
	EGameState GetCurrentState() const { return CurrentState; }
	
	protected:
	virtual void BeginPlay() override;
	
	// --- Game mode ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<ASnakePawn> SnakePawnClass;
	
	UPROPERTY(EditDefaultsOnly, Category="GameMode")
	FTransform Player1SpawnTransform;
	
	UPROPERTY(EditDefaultsOnly, Category="GameMode")
	FTransform Player2SpawnTransform;
	
	
	// Player1IMC = WASD mapping context
	// Player2IMC = Arrow Keys mapping context
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UInputMappingContext> Player1IMC;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInputMappingContext> Player2IMC;
 
	// Widget to show on a player's screen when they lose
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UUserWidget> PlayerGameOverWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, Category="GameMode")
	float RestartDelay = 3.0f;
	
	// --- Grid ---
	UPROPERTY(EditDefaultsOnly, Category="Grid")
	TSubclassOf<AActor> TileBP;
	
	UPROPERTY(EditDefaultsOnly, Category="Grid")
	float TileSize = 100.f;
	
	TArray<AActor*> TileActors;
	
	void SpawnTiles();
	void RefreshTile(FIntPoint Pos);
	
	private:
	EGameState CurrentState = EGameState::MainMenu;
	FTimerHandle RestartTimer;
	
	UPROPERTY()
	int32 NumPlayers = 1;
	
	int32 ActivePlayerCount = 0; // How many snakes are still alive
	
	void SpawnAndPossessPlayers();
	void SetGameState(EGameState newState);

};
