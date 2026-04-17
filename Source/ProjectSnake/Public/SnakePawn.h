
#pragma once

#include "CoreMinimal.h"       
#include "InputActionValue.h"
#include "GameFramework/Pawn.h"
#include "SnakeSegment.h"
#include "SnakeHUD.h"          
#include "SnakeGrid.h"
#include "SnakePawn.generated.h" 

class UStaticMeshComponent;
class USphereComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;

UCLASS() // Tells Unreal this class participates in the reflection/object system
class PROJECTSNAKE_API ASnakePawn : public APawn
{
	GENERATED_BODY() // Injects generated boilerplate that Unreal needs

public:
	// Sets default values for this pawn's properties
	ASnakePawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void Move(const FInputActionValue& Value);
	void Turn(const FInputActionValue& Value);
	
#pragma region SNAKEPROPERTIES
	// -------------------- SNAKE PAWN PROPERTIES -----------------------------------
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	TObjectPtr<UCameraComponent> CameraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> TurnAction;
	
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> BoostAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float MoveSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float TurnSpeed = 200.0f;
	
	// --------------------- SNAKE SEGMENT PROPERTIES ---------------------------------
	
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
	
	// ---------------------- SNAKE POWERS ---------------------------------------------
	
	// Active power state
	bool bIsInvisible = false;
	float ActiveSpeedMultiplier = 1.0f;
	
	// Timer for power duration
	FTimerHandle InvisibilityTimer;
	FTimerHandle SpeedBoostTimer;
	
	// Power appliers
	void ApplySpeedBoost(float Multiplier, float Duration);
	void ApplyInvisibility(float Duration);

	// Power removers
	void RemoveSpeedBoost();
	void RemoveInvisibility();
	
	// --------------------- SNAKE BOOST ---------------------------------------------
	
	bool bWantsToBoost = false; // boost input
	
	// Boost bar (0.0 = empty, 1.0 = full)
	UPROPERTY(EditDefaultsOnly, Category="Boost")
	float BoostCharge = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, Category="Boost")
	float BoostDrainRate = 0.2f; // Per second while boosting
	
	UPROPERTY(EditDefaultsOnly, Category="Boost")
	float BoostRefillRate = 0.1f; // Per second while not boosting
	
	UPROPERTY(EditDefaultsOnly, Category="Boost")
	float BoostSpeedMultiplier = 1.8f; // How fast during boost
	
	bool bUnlimitedBoost = false; // Speed boost for food type
	FTimerHandle UnlimitedBoostTimer; // Speed boost timer
	
	UFUNCTION(BlueprintCallable)
	float GetBoostCharge() const { return BoostCharge; }
	
	UFUNCTION(BlueprintCallable)
	bool IsUnlimitedBoost() const { return bUnlimitedBoost;	}
	
	void ApplyUnlimitedBoost(float Duration);
	void RemoveUnlimitedBoost();
	
	// ----------------------- GRID -----------------------------------------------------
	
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
	// ----------------------------------------------------------------------------------------
#pragma endregion
	
#pragma region HUDPROPERTIES
	// ----------------------- HUD --------------------------------------------------------
	
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;
	
	USnakeHUD* HUDWidget;
	
	void UpdateHUDScore() const;
	
	int32 Score = 0;
	
	//----------------------------------------------------------------------------------------
#pragma endregion
	
	// Hit event for food overlap
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
	
	// Hit event for wall collision
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	void AddSegment();
	void SetupSegmentPositions();
	
	void OnBoostPressed();
	void OnBoostReleased();
	
	void UpdateHUDBoost() const; 

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	float MoveInput = 0.0f;
	float TurnInput = 0.0f;
	
	int32 LastMaterialIndex = -1;
    UMaterialInstance* GetNextMaterial();
};