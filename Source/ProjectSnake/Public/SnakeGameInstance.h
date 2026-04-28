#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SnakeGameInstance.generated.h"

UENUM(BlueprintType)
enum class ESnakeGameMode : uint8
{
	SinglePlayer  UMETA(DisplayName = "Single Player"),
	Multiplayer   UMETA(DisplayName = "Multiplayer"),
};

UCLASS()
class PROJECTSNAKE_API USnakeGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	// Set by the Main Menu before loading the game level.
	UPROPERTY(BlueprintReadWrite, Category="Game")
	ESnakeGameMode SelectedMode = ESnakeGameMode::SinglePlayer;
};