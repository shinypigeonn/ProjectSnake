// Fill out your copyright notice in the Description page of Project Settings.

#include "SnakeGameMode.h"
#include "SnakePawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

ASnakeGameMode::ASnakeGameMode()
{
	// Disable the default single-pawn auto-spawn so we control it ourselves
	DefaultPawnClass = nullptr;
}

void ASnakeGameMode::BeginPlay()
{
	Super::BeginPlay();
	SpawnAndPossessPlayers();
	SetGameState(EGameState::Playing);
}

//  ────────────────────────────────────────────────────────────────────────────────────────────────────────
// SETUP
//  ────────────────────────────────────────────────────────────────────────────────────────────────────────
#pragma region SETUP | IMC & LOCAL PLAYER SETUP

void ASnakeGameMode::SpawnAndPossessPlayers()
{
	if (!SnakePawnClass) return;
	UWorld* World = GetWorld();
	if (!World) return;
	
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	// --- Player 1 ---
	APlayerController* PC0 = UGameplayStatics::GetPlayerController(World, 0);
	if (!PC0)
		PC0 = UGameplayStatics::CreatePlayer(World, 0, false);
 
	if (PC0)
	{
		ASnakePawn* Pawn1 = World->SpawnActor<ASnakePawn>(SnakePawnClass, Player1SpawnTransform, Params);
		if (Pawn1)
		{
			Pawn1->ApplyIMC(Player1IMC); // WASD
			ActivePlayerCount++;
			PC0->Possess(Pawn1);
		}
	}
	
	// --- Player 2 ---
	APlayerController* PC1 = UGameplayStatics::GetPlayerController(World, 1);
	if (!PC1)
		PC1 = UGameplayStatics::CreatePlayer(World, 1, false);
 
	if (PC1)
	{
		ASnakePawn* Pawn2 = World->SpawnActor<ASnakePawn>(SnakePawnClass, Player2SpawnTransform, Params);
		if (Pawn2)
		{
			Pawn2->ApplyIMC(Player2IMC); // Gamepad
			ActivePlayerCount++;	
			PC1->Possess(Pawn2);
		}
	}
}

#pragma endregion

//  ────────────────────────────────────────────────────────────────────────────────────────────────────────
// PER-PLAYER DEATH — only affects the player who died
//  ────────────────────────────────────────────────────────────────────────────────────────────────────────
void ASnakeGameMode::OnPlayerDied(ASnakePawn* DeadPawn)
{
	if (!DeadPawn || CurrentState != EGameState::Playing) return;
 
	// Find which controller owns this pawn
	APlayerController* PC = Cast<APlayerController>(DeadPawn->GetController());
	if (PC)
	{
		// Disable input so the dead snake stops moving
		PC->DisableInput(PC);
 
		// Show a "You Lose" widget on just this player's screen
		if (PlayerGameOverWidgetClass)
		{
			UUserWidget* Widget = CreateWidget<UUserWidget>(PC, PlayerGameOverWidgetClass);
			if (Widget) Widget->AddToPlayerScreen(); // AddToPlayerScreen = this viewport only
		}
	}
 
	// Destroy the dead snake
	DeadPawn->Destroy();
 
	// Track how many players are left
	ActivePlayerCount = FMath::Max(0, ActivePlayerCount - 1);
 
	// If everyone is dead, do a full game over after a short delay
	if (ActivePlayerCount <= 0)
	{
		SetGameState(EGameState::GameOver);
		GetWorldTimerManager().SetTimer(RestartTimer, this, &ASnakeGameMode::RestartGame, RestartDelay, false);
	}
}

//  ────────────────────────────────────────────────────────────────────────────────────────────────────────
// GAME STATES
//  ────────────────────────────────────────────────────────────────────────────────────────────────────────
#pragma region GAME STATES
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

#pragma endregion