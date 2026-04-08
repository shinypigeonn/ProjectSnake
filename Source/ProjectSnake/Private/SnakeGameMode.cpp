// Fill out your copyright notice in the Description page of Project Settings.

#include "SnakeGameMode.h"
#include "Kismet/GameplayStatics.h"

ASnakeGameMode::ASnakeGameMode()
{
}

void ASnakeGameMode::BeginPlay()
{
	Super::BeginPlay();
	SetGameState(EGameState::Playing);
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