// Fill out your copyright notice in the Description page of Project Settings.

#include "SnakePlayerController.h"
#include "SnakePawn.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

// ────────────────────────────────────────────────────────────────────────────────
// INITIALIZATION
// ────────────────────────────────────────────────────────────────────────────────

void ASnakePlayerController::BeginPlay()
{
	Super::BeginPlay();
	RegisterIMC();
}

void ASnakePlayerController::RegisterIMC()
{
	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsystem->ClearAllMappings();
			if (InputMapping)
				Subsystem->AddMappingContext(InputMapping, 0);
		}
	}
}

// Called by GameMode for Player 2 to swap in Arrow Keys IMC and actions.
// Rebinds actions immediately so timing doesn't matter.
void ASnakePlayerController::SetupPlayerInput(
	UInputMappingContext* IMC,
	UInputAction* MoveAction,
	UInputAction* TurnAction,
	UInputAction* BoostAction)
{
	InputMapping = IMC;
	IA_Move  = MoveAction;
	IA_Turn  = TurnAction;
	IA_Boost = BoostAction;

	// Re-register IMC
	RegisterIMC();

	// Rebind actions — SetupInputComponent already ran, so we need to
	// clear and re-add bindings with the new action assets.
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EIC->ClearActionBindings();

		if (IA_Move)
		{
			EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &ASnakePlayerController::Move);
			EIC->BindAction(IA_Move, ETriggerEvent::Completed, this, &ASnakePlayerController::Move);
		}
		if (IA_Turn)
		{
			EIC->BindAction(IA_Turn, ETriggerEvent::Triggered, this, &ASnakePlayerController::Turn);
			EIC->BindAction(IA_Turn, ETriggerEvent::Completed, this, &ASnakePlayerController::Turn);
		}
		if (IA_Boost)
		{
			EIC->BindAction(IA_Boost, ETriggerEvent::Triggered,  this, &ASnakePlayerController::OnBoostPressed);
			EIC->BindAction(IA_Boost, ETriggerEvent::Completed,  this, &ASnakePlayerController::OnBoostReleased);
		}
	}
}

// ────────────────────────────────────────────────────────────────────────────────
// INPUT BINDING
// ────────────────────────────────────────────────────────────────────────────────

void ASnakePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (IA_Move)
		{
			EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &ASnakePlayerController::Move);
			EIC->BindAction(IA_Move, ETriggerEvent::Completed, this, &ASnakePlayerController::Move);
		}
		if (IA_Turn)
		{
			EIC->BindAction(IA_Turn, ETriggerEvent::Triggered, this, &ASnakePlayerController::Turn);
			EIC->BindAction(IA_Turn, ETriggerEvent::Completed, this, &ASnakePlayerController::Turn);
		}
		if (IA_Boost)
		{
			EIC->BindAction(IA_Boost, ETriggerEvent::Triggered,  this, &ASnakePlayerController::OnBoostPressed);
			EIC->BindAction(IA_Boost, ETriggerEvent::Completed,  this, &ASnakePlayerController::OnBoostReleased);
		}
	}
}

// ────────────────────────────────────────────────────────────────────────────────
// INPUT FORWARDING TO PAWN
// ────────────────────────────────────────────────────────────────────────────────

ASnakePawn* ASnakePlayerController::GetSnakePawn() const
{
	return Cast<ASnakePawn>(GetPawn());
}

void ASnakePlayerController::Move(const FInputActionValue& Value)
{
	if (ASnakePawn* SnakePawn = GetSnakePawn())
		SnakePawn->Move(Value);
}

void ASnakePlayerController::Turn(const FInputActionValue& Value)
{
	if (ASnakePawn* SnakePawn = GetSnakePawn())
		SnakePawn->Turn(Value);
}

void ASnakePlayerController::OnBoostPressed()
{
	if (ASnakePawn* SnakePawn = GetSnakePawn())
		SnakePawn->OnBoostPressed();
}

void ASnakePlayerController::OnBoostReleased()
{
	if (ASnakePawn* SnakePawn = GetSnakePawn())
		SnakePawn->OnBoostReleased();
}