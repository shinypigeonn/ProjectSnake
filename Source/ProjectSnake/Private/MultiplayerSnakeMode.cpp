#include "MultiplayerSnakeMode.h"
#include "SnakePawn.h"
#include "SnakeGameInstance.h"
#include "SnakePlayerController.h"
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
    
    UE_LOG(LogTemp, Warning, TEXT("=== MultiplayerSnakeMode BeginPlay ==="));
    
    if (USnakeGameInstance* GI = Cast<USnakeGameInstance>(GetGameInstance()))
    {
        UE_LOG(LogTemp, Warning, TEXT("GameInstance cast SUCCESS, mode: %d"), (int32)GI->SelectedMode);
        bIsMultiplayer = (GI->SelectedMode == ESnakeGameMode::Multiplayer);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("GameInstance cast FAILED"));
    }

    UE_LOG(LogTemp, Warning, TEXT("bIsMultiplayer: %d"), bIsMultiplayer);
    UE_LOG(LogTemp, Warning, TEXT("SnakePawnClass is: %s"), SnakePawnClass ? *SnakePawnClass->GetName() : TEXT("NULL"));

    SpawnPlayers();
    StartCountdown();
}

// ────────────────────────────────────────────────────────────────────────────────
// SPAWNING
// ────────────────────────────────────────────────────────────────────────────────

static FTransform ResolveSpawn(UWorld* World, const FTransform& Explicit, int32 Index)
{
	if (!Explicit.GetLocation().IsNearlyZero()) return Explicit;
	TArray<AActor*> Starts;
	UGameplayStatics::GetAllActorsOfClass(World, APlayerStart::StaticClass(), Starts);
	if (Starts.IsValidIndex(Index)) return Starts[Index]->GetActorTransform();
	return FTransform::Identity;
}

void AMultiplayerSnakeMode::SpawnPlayers()
{
	if (bIsMultiplayer)
		SpawnMultiplayer();
	else
		SpawnSinglePlayer();
}

void AMultiplayerSnakeMode::SpawnSinglePlayer()
{

    UE_LOG(LogTemp, Warning, TEXT("=== SpawnSinglePlayer called ==="));
    
    if (!SnakePawnClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("FAILED: SnakePawnClass is null"));
        return;
    }
 
	UWorld* World = GetWorld();

	APlayerController* PC0 = UGameplayStatics::GetPlayerController(World, 0);
	if (!PC0) PC0 = UGameplayStatics::CreatePlayer(World, 0, false);
	if (!PC0) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ASnakePawn* Pawn = World->SpawnActor<ASnakePawn>(
		SnakePawnClass, ResolveSpawn(World, Player1SpawnTransform, 0), Params);
	if (Pawn)
	{
		PC0->Possess(Pawn);
		ActivePlayerCount++;
	}
}

void AMultiplayerSnakeMode::SpawnMultiplayer()
{
	if (!SnakePawnClass) return;
	UWorld* World = GetWorld();

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// --- Player 1 ---
	APlayerController* PC0 = UGameplayStatics::GetPlayerController(World, 0);
	if (!PC0) PC0 = UGameplayStatics::CreatePlayer(World, 0, false);
	if (PC0)
	{
		ASnakePawn* Pawn1 = World->SpawnActor<ASnakePawn>(
			SnakePawnClass, ResolveSpawn(World, Player1SpawnTransform, 0), Params);
		if (Pawn1)
		{
			PC0->Possess(Pawn1);
			ActivePlayerCount++;
		}
	}

	// --- Player 2 — create second local player (enables split screen) ---
	APlayerController* PC1 = UGameplayStatics::GetPlayerController(World, 1);
	if (!PC1) PC1 = UGameplayStatics::CreatePlayer(World, 1, true); // true = show splitscreen
	if (PC1)
	{
		ASnakePawn* Pawn2 = World->SpawnActor<ASnakePawn>(
			SnakePawnClass, ResolveSpawn(World, Player2SpawnTransform, 1), Params);
		if (Pawn2)
		{
			if (ASnakePlayerController* SC = Cast<ASnakePlayerController>(PC1))
				SC->SetupPlayerInput(Player2IMC, Player2_IA_Move, Player2_IA_Turn, Player2_IA_Boost);
			PC1->Possess(Pawn2);
			ActivePlayerCount++;
		}
	}
}

// ────────────────────────────────────────────────────────────────────────────────
// COUNTDOWN
// ────────────────────────────────────────────────────────────────────────────────

void AMultiplayerSnakeMode::StartCountdown()
{
	SetGameState(EGameState::Countdown);
	CountdownSecondsRemaining = CountdownSeconds;

	// Disable movement on all pawns during countdown
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
			PC->DisableInput(PC);
	}

	GetWorldTimerManager().SetTimer(
		CountdownTimer, this, &AMultiplayerSnakeMode::CountdownTick, 1.0f, true);
}

void AMultiplayerSnakeMode::CountdownTick()
{
	CountdownSecondsRemaining--;

	if (CountdownSecondsRemaining <= 0)
	{
		GetWorldTimerManager().ClearTimer(CountdownTimer);
		StartPlay();
	}
	// The HUD widget can poll GetCountdownSeconds() via Blueprint to show the number.
}

void AMultiplayerSnakeMode::StartPlay()
{
	SetGameState(EGameState::Playing);

	// Re-enable input on all pawns
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
			PC->EnableInput(PC);
	}
}

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

void AMultiplayerSnakeMode::SetGameState(EGameState NewState) { CurrentState = NewState; }

void AMultiplayerSnakeMode::OnGameOver()
{
	if (CurrentState != EGameState::Playing) return;
	SetGameState(EGameState::GameOver);
	UGameplayStatics::OpenLevel(this, FName("MainMenuMap"), true);
}

void AMultiplayerSnakeMode::RestartGame()
{
	SetGameState(EGameState::Restarting);
	UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), true);
}