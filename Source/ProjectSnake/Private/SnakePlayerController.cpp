// SnakePlayerController.cpp

#include "SnakePlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

void ASnakePlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (!GetLocalPlayer()) return;

    int32 ID = GetLocalPlayer()->GetControllerId();

    UInputMappingContext* ChosenIMC = nullptr;

    if (ID == 0)
    {
        ChosenIMC = Player1IMC; // WASD
    }
    else if (ID == 1)
    {
        ChosenIMC = Player2IMC; // Arrows
    }

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
        GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
    {
        if (ChosenIMC)
        {
            Subsystem->AddMappingContext(ChosenIMC, 0);
        }
    }
}