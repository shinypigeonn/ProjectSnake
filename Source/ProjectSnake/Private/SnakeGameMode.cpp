// Fill out your copyright notice in the Description page of Project Settings.

#include "SnakeGameMode.h"
#include "Kismet/GameplayStatics.h"

ASnakeGameMode::ASnakeGameMode()
{
}

void ASnakeGameMode::BeginPlay()
{
	Super::BeginPlay();
	Grid = FSnakeGrid(50, 50);
	SpawnTiles();
	SetGameState(EGameState::Playing);
}

void ASnakeGameMode::SpawnTiles() // CAN BE REMOVED
{
	/*for (int32 y = 0; y < Grid.Height; y++)
	{
		for (int32 X = 0; x < Grid.Width; x++)
		{
			//FVector Pos(X * TileSize, Y * TileSize, 0.f);
            //AActor* Tile = GetWorld()->SpawnActor<AActor>(TileBP, Pos, FRotator::ZeroRotator);
            //TileActors.Add(Tile);
		}
	}*/
}
void ASnakeGameMode::SetGameState(EGameState NewState)
{
	CurrentState = NewState;
}

void ASnakeGameMode::MainMenu()
{
    SetGameState(EGameState::MainMenu);
    UGameplayStatics::OpenLevel(this, FName("MainMenuMap"), true);
}

void ASnakeGameMode::OnPlay()
{
    if (CurrentState != EGameState::WaitingToStart) return;
    SetGameState(EGameState::Playing);
}

void ASnakeGameMode::OnGameOver()
{
	if (CurrentState != EGameState::Playing) return; // Prevent duplicate calls

	SetGameState(EGameState::GameOver);
	UGameplayStatics::OpenLevel(this, FName("GameOverMap"), true);
}

void ASnakeGameMode::RestartGame()
{
	SetGameState(EGameState::Restarting);
	//UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), true);
}