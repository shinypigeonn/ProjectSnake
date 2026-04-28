#pragma once

#include "CoreMinimal.h"       
#include "InputActionValue.h"
#include "GameFramework/Pawn.h"
#include "SnakeSegment.h"
#include "SnakeHUD.h"          
#include "SnakeGrid.h"
#include "SnakePawn.generated.h" 

class UUserWidget;
class UStaticMeshComponent;
class USphereComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;

UCLASS()
class PROJECTSNAKE_API ASnakePawn : public APawn
{
	GENERATED_BODY()

public:
	ASnakePawn();

	// Called after possession is complete — safe point to register input
	virtual void PawnClientRestart() override;

	// Called by GameMode to inject the correct IMC per player before possession
	void ApplyIMC(UInputMappingContext* IMC);

protected:
	virtual void BeginPlay() override;

	// --- Input ---
	void Move(const FInputActionValue& Value);
	void Turn(const FInputActionValue& Value);
	void OnBoostPressed();
	void OnBoostReleased();
	
#pragma region SNAKEPROPERTIES

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	TObjectPtr<UCameraComponent> CameraComponent;

	// Fallback IMC set in Blueprint defaults (used only if GameMode doesn't call ApplyIMC)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> IA_Move;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> IA_Turn;
	
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> IA_Boost;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float MoveSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float TurnSpeed = 200.0f;
	
	// --- Snake Segment Properties ---
	
	UPROPERTY(EditDefaultsOnly, Category="Snake")
	TSubclassOf<ASnakeSegment> SegmentClass;

	UPROPERTY(VisibleAnywhere, Category="Snake")
	TArray<ASnakeSegment*> Segments;

	UPROPERTY(VisibleAnywhere, Category="Snake")
	TArray<FVector> PositionHistory;

	UPROPERTY(EditDefaultsOnly, Category="Snake")
	int32 SegmentSpacing = 60;
	
	UPROPERTY(EditAnywhere)
	int32 InitialSegments = 3;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ASnakeSegment> SnakeSegment;
	
	UPROPERTY(EditDefaultsOnly, Category="Snake")
	TArray<TObjectPtr<UMaterialInstance>> WatercolorMaterials;
	
	// --- Snake Powers ---
	
	bool bIsInvisible = false;
	float ActiveSpeedMultiplier = 1.0f;
	
	FTimerHandle InvisibilityTimer;
	FTimerHandle SpeedBoostTimer;
	
	void ApplySpeedBoost(float Multiplier, float Duration);
	void ApplyInvisibility(float Duration);
	void RemoveSpeedBoost();
	void RemoveInvisibility();
	
	// --- Snake Boost ---
	
	bool bWantsToBoost = false;
	
	UPROPERTY(EditDefaultsOnly, Category="Boost")
	float BoostCharge = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, Category="Boost")
	float BoostDrainRate = 0.2f;
	
	UPROPERTY(EditDefaultsOnly, Category="Boost")
	float BoostRefillRate = 0.1f;
	
	UPROPERTY(EditDefaultsOnly, Category="Boost")
	float BoostSpeedMultiplier = 1.8f;
	
	bool bUnlimitedBoost = false;
	FTimerHandle UnlimitedBoostTimer;
	
	UFUNCTION(BlueprintCallable)
	float GetBoostCharge() const { return BoostCharge; }
	
	UFUNCTION(BlueprintCallable)
	bool IsUnlimitedBoost() const { return bUnlimitedBoost; }
	
	void ApplyUnlimitedBoost(float Duration);
	void RemoveUnlimitedBoost();
	
	// --- Grid ---
	
	FSnakeGrid Grid;
	
	UPROPERTY(EditAnywhere, Category="Grid")
	int32 GridWidth = 20;

	UPROPERTY(EditAnywhere, Category="Grid")
	int32 GridHeight = 20;
	
	UPROPERTY(EditAnywhere, Category="Grid")
	float CellSize = 100.0f;
	
	FIntPoint WorldToGrid(FVector WorldPos) const;
	FVector GridToWorld(FIntPoint GridPos) const;
	FVector GetRandomEmptyCell() const;

#pragma endregion
	
#pragma region HUDPROPERTIES

	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;
	
	USnakeHUD* HUDWidget;
	
	void UpdateHUDScore() const;
	
	int32 Score = 0;

#pragma endregion
	
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	
	void AddSegment();
	void SetupSegmentPositions();
	
	// --- Tick Helpers ---
	void UpdateBoostState(float DeltaTime);	
	void UpdateMovement(float DeltaTime);
	void UpdateSegments();	
	void UpdateGrid();
	void UpdateHUDBoost() const;
	
	// --- Initialization ---
	void InitGrid();
	void InitSnake();
	void InitHUD();

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintPure, Category="Score")
	int32 GetScore() const { return Score; }
	
private:
	float MoveInput = 0.0f;
	float TurnInput = 0.0f;
	
	int32 LastMaterialIndex = -1;
	UMaterialInstance* GetNextMaterial();

	// IMC injected by GameMode — takes priority over the Blueprint default InputMappingContext
	UPROPERTY()
	TObjectPtr<UInputMappingContext> AssignedIMC;
};