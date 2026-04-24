
#pragma once

#include "CoreMinimal.h"
#include "SnakeGrid.h"
#include "InputMappingContext.h"
#include "GameFramework/GameMode.h"
#include "MultiplayerSnakeMode.generated.h"

class ASnakePawn;

UCLASS()
class PROJECTSNAKE_API AMultiplayerSnakeMode : public AGameMode
{
	GENERATED_BODY()
	
	FSnakeGrid Grid;
	
	public:
	AMultiplayerSnakeMode();

	UFUNCTION(BlueprintCallable, Category="GameMode")
	void OnPlayerDied(ASnakePawn* DeadPawn);

	protected:
	virtual void BeginPlay() override;
	
	// --- Game mode ---
	UPROPERTY(EditDefaultsOnly, Category="GameMode")
	TSubclassOf<ASnakePawn> SnakePawnClass;
	
	UPROPERTY(EditDefaultsOnly, Category="GameMode")
	FTransform Player1SpawnTransform;
	
	UPROPERTY(EditDefaultsOnly, Category="GameMode")
	FTransform Player2SpawnTransform;
	
	// Player1IMC = WASD mapping context
	// Player2IMC = Arrow Keys mapping context
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
    TObjectPtr<UInputMappingContext> Player1IMC;
	
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputMappingContext> Player2IMC;
 
	// Widget to show on a player's screen when they lose
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UUserWidget> PlayerGameOverWidgetClass;
	
	int32 ActivePlayerCount = 0; // How many snakes are still alive
	
	void SpawnAndPossessPlayers();
};
