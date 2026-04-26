// Fill out your copyright notice in the Description page of Project Settings.

#include "MultiplayerSnakeMode.h"
#include "SnakePawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

AMultiplayerSnakeMode::AMultiplayerSnakeMode()
{
	DefaultPawnClass = nullptr;
}

void AMultiplayerSnakeMode::BeginPlay()
{
	Super::BeginPlay();
	SpawnAndPossessPlayers();
	SetGameState(EGameState::Playing);
}

// ────────────────────────────────────────────────────────────────────────────────
// SETUP
// ────────────────────────────────────────────────────────────────────────────────
#pragma region SETUP

static FTransform ResolveSpawnTransform(UWorld* World, const FTransform& Explicit, int32 PlayerIndex)
{
	if (!Explicit.GetLocation().IsNearlyZero())
		return Explicit;

	TArray<AActor*> Starts;
	UGameplayStatics::GetAllActorsOfClass(World, APlayerStart::StaticClass(), Starts);
	if (Starts.IsValidIndex(PlayerIndex))
		return Starts[PlayerIndex]->GetActorTransform();

	UE_LOG(LogTemp, Warning, TEXT("MultiplayerSnakeMode: No spawn point for Player %d."), PlayerIndex);
	return FTransform::Identity;
}

void AMultiplayerSnakeMode::SpawnAndPossessPlayers()
{
	if (!SnakePawnClass) return;
	UWorld* World = GetWorld();
	if (!World) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// --- Player 1 ---
	APlayerController* PC0 = UGameplayStatics::GetPlayerController(World, 0);
	if (!PC0) PC0 = UGameplayStatics::CreatePlayer(World, 0, false);
	if (PC0)
	{
		ASnakePawn* Pawn1 = World->SpawnActor<ASnakePawn>(
			SnakePawnClass, ResolveSpawnTransform(World, Player1SpawnTransform, 0), Params);
		if (Pawn1)
		{
			// ApplyIMC must be called BEFORE Possess so PawnClientRestart sees the right IMC
			Pawn1->ApplyIMC(Player1IMC);
			PC0->Possess(Pawn1);
			ActivePlayerCount++;
		}
	}

	// --- Player 2 ---
	APlayerController* PC1 = UGameplayStatics::GetPlayerController(World, 1);
	if (!PC1) PC1 = UGameplayStatics::CreatePlayer(World, 1, false);
	if (PC1)
	{
		ASnakePawn* Pawn2 = World->SpawnActor<ASnakePawn>(
			SnakePawnClass, ResolveSpawnTransform(World, Player2SpawnTransform, 1), Params);
		if (Pawn2)
		{
			Pawn2->ApplyIMC(Player2IMC);
			PC1->Possess(Pawn2);
			ActivePlayerCount++;
		}
	}
}

#pragma endregion

// ────────────────────────────────────────────────────────────────────────────────
// PER-PLAYER DEATH
// ────────────────────────────────────────────────────────────────────────────────

void AMultiplayerSnakeMode::OnPlayerDied(ASnakePawn* DeadPawn)
{
	if (!DeadPawn || CurrentState != EGameState::Playing) return;

	APlayerController* PC = Cast<APlayerController>(DeadPawn->GetController());
	if (PC)
	{
		PC->DisableInput(PC);
		if (PlayerGameOverWidgetClass)
		{
			UUserWidget* Widget = CreateWidget<UUserWidget>(PC, PlayerGameOverWidgetClass);
			if (Widget) Widget->AddToPlayerScreen();
		}
	}

	DeadPawn->Destroy();
	ActivePlayerCount = FMath::Max(0, ActivePlayerCount - 1);

	if (ActivePlayerCount <= 0)
	{
		SetGameState(EGameState::GameOver);
		GetWorldTimerManager().SetTimer(
			RestartTimer, this, &AMultiplayerSnakeMode::RestartGame, RestartDelay, false);
	}
}

// ────────────────────────────────────────────────────────────────────────────────
// GAME STATES
// ────────────────────────────────────────────────────────────────────────────────

void AMultiplayerSnakeMode::SetGameState(EGameState NewState)
{
	CurrentState = NewState;
}

void AMultiplayerSnakeMode::OnGameOver()
{
	if (CurrentState != EGameState::Playing) return;
	SetGameState(EGameState::GameOver);
	UGameplayStatics::OpenLevel(this, FName("GameOverMap"), true);
}

void AMultiplayerSnakeMode::RestartGame()
{
	SetGameState(EGameState::Restarting);
	UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), true);
}