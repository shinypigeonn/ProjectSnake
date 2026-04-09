
#pragma once

#include "CoreMinimal.h"       
#include "InputActionValue.h"
#include "GameFramework/Pawn.h"
#include "SnakeSegment.h"
#include "SnakeHUD.h"          
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
	// ----------------- SNAKE PAWN PROPERTIES --------------------------
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float MoveSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float TurnSpeed = 200.0f;
	
	// ---------------- SNAKE SEGMENT PROPERTIES  ----------------------
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
#pragma endregion
	
	// ----------------------- HUD ---------------------------------
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;
	UPROPERTY()
	USnakeHUD* HUDWidget;
	
	void UpdateHUDScore();
	
	int32 Score = 0;
	//---------------------------------------------------------------
	
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

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	float MoveInput = 0.0f;
	float TurnInput = 0.0f;
};