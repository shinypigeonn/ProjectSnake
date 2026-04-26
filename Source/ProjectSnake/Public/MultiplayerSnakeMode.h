#pragma once

#include "CoreMinimal.h"
#include "SnakeGrid.h"
#include "InputMappingContext.h"
#include "GameFramework/GameMode.h"
#include "MultiplayerSnakeMode.generated.h"

class ASnakePawn;

UENUM(BlueprintType)
enum class EGameState : uint8
{
	MainMenu       UMETA(DisplayName = "Main Menu"),
	WaitingToStart UMETA(DisplayName = "Waiting to Start"),
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

	// AGameMode already declares RestartGame() as a UFUNCTION — just override it.
	virtual void RestartGame() override;

	UFUNCTION(BlueprintPure, Category="GameMode")
	EGameState GetCurrentState() const { return CurrentState; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category="GameMode")
	TSubclassOf<ASnakePawn> SnakePawnClass;

	UPROPERTY(EditDefaultsOnly, Category="GameMode")
	FTransform Player1SpawnTransform;

	UPROPERTY(EditDefaultsOnly, Category="GameMode")
	FTransform Player2SpawnTransform;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputMappingContext> Player1IMC;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputMappingContext> Player2IMC;

	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UUserWidget> PlayerGameOverWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="GameMode")
	float RestartDelay = 3.0f;

private:
	EGameState CurrentState = EGameState::MainMenu;
	FTimerHandle RestartTimer;
	int32 ActivePlayerCount = 0;

	void SpawnAndPossessPlayers();
	void SetGameState(EGameState NewState);
};