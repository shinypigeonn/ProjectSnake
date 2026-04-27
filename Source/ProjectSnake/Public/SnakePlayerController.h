#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "SnakePlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class ASnakePawn;

UCLASS()
class PROJECTSNAKE_API ASnakePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	// Called by GameMode to assign which IMC and actions this controller should use.
	// Call this before or right after possession.
	void SetupPlayerInput(
		UInputMappingContext* IMC,
		UInputAction* MoveAction,
		UInputAction* TurnAction,
		UInputAction* BoostAction);
	
	void Move(const FInputActionValue& Value);
	void Turn(const FInputActionValue& Value);
	void OnBoostPressed();
	void OnBoostReleased();
	
protected:
	// Set in Blueprint for Player 1 (WASD) — used as default if GameMode doesn't override
	UPROPERTY(EditDefaultsOnly, Category="Enhanced Input")
	TObjectPtr<UInputMappingContext> InputMapping;

	UPROPERTY(EditDefaultsOnly, Category="Enhanced Input")
	TObjectPtr<UInputAction> IA_Move;

	UPROPERTY(EditDefaultsOnly, Category="Enhanced Input")
	TObjectPtr<UInputAction> IA_Turn;

	UPROPERTY(EditDefaultsOnly, Category="Enhanced Input")
	TObjectPtr<UInputAction> IA_Boost;

private:
	ASnakePawn* GetSnakePawn() const;
};