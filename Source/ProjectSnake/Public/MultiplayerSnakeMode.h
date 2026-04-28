#pragma once

#include "CoreMinimal.h"
#include "InputMappingContext.h"
#include "GameFramework/GameMode.h"
#include "MultiplayerSnakeMode.generated.h"

class ASnakePawn;
class UInputAction;

UENUM(BlueprintType)
enum class EGameState : uint8
{
	MainMenu       UMETA(DisplayName = "Main Menu"),
	Countdown      UMETA(DisplayName = "Countdown"),
	Playing        UMETA(DisplayName = "Playing"),
	GameOver       UMETA(DisplayName = "Game Over"),
	Restarting     UMETA(DisplayName = "Restarting"),
};

UCLASS()
class PROJECTSNAKE_API AMultiplayerSnakeMode : public AGameMode
{
	GENERATED_BODY()

public:
	AMultiplayerSnakeMode();

	UFUNCTION(BlueprintCallable, Category="GameMode")
	void OnPlayerDied(ASnakePawn* DeadPawn);

	UFUNCTION(BlueprintCallable, Category="GameMode")
	void OnGameOver();

	virtual void RestartGame() override;

	UFUNCTION(BlueprintPure, Category="GameMode")
	EGameState GetCurrentState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category="GameMode")
	int32 GetCountdownSeconds() const { return CountdownSecondsRemaining; }

protected:
	virtual void BeginPlay() override;

	// --- Spawning ---
	UPROPERTY(EditDefaultsOnly, Category="GameMode")
	TSubclassOf<ASnakePawn> SnakePawnClass;

	UPROPERTY(EditDefaultsOnly, Category="GameMode")
	FTransform Player1SpawnTransform;

	UPROPERTY(EditDefaultsOnly, Category="GameMode")
	FTransform Player2SpawnTransform;

	// --- Input (Player 2 only — Player 1 set on BP_SnakePlayerController) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInputMappingContext> Player2IMC;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInputAction> Player2_IA_Move;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInputAction> Player2_IA_Turn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInputAction> Player2_IA_Boost;

	// --- UI ---
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UUserWidget> PlayerGameOverWidgetClass;

	// --- Countdown ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CountdownSeconds = 3;

	// --- Restart ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RestartDelay = 3.0f;

private:
	EGameState CurrentState = EGameState::MainMenu;
	FTimerHandle RestartTimer;
	FTimerHandle CountdownTimer;
	int32 ActivePlayerCount = 0;
	int32 CountdownSecondsRemaining = 0;
	bool bIsMultiplayer = false;

	void SpawnPlayers();
	void SpawnSinglePlayer();
	void SpawnMultiplayer();
	void StartCountdown();
	void CountdownTick();
	void StartPlay();
	void SetGameState(EGameState NewState);
};