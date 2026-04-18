// Fill out your copyright notice in the Description page of Project Settings

#include "SnakePawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "InputAction.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Food.h"
#include "SnakeGameMode.h"

// Sets default values
ASnakePawn::ASnakePawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// --- Collision Components ---
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	SetRootComponent(CollisionComponent);
	CollisionComponent->InitSphereRadius(50.0f);
	
	// --- Mesh Components ---
	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(CollisionComponent);
	
	// --- SpringArm Component ---
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(CollisionComponent);
	SpringArm->TargetArmLength = 700.0f;
	SpringArm->SetRelativeRotation(FRotator(-55.0f, 0.0f, 0.0f));
	SpringArm->bDoCollisionTest = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = false;
	SpringArm->bInheritYaw = false;

	// --- Camera component ---
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArm);

	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

// Called when the game starts or when spawned
void ASnakePawn::BeginPlay()
{
	// Runtime intialization, adding input mapping contexts, reference that depend on having a world
	Super::BeginPlay();
	
	Grid = FSnakeGrid(GridWidth, GridHeight);
	
	// Mark border cells as walls
	for (int32 X = 0; X < GridWidth; X++)
	{
		for (int32 Y = 0; Y < GridHeight; Y++)
		{
			bool bIsBorder = X == 0 || X == GridWidth - 1
                      || Y == 0 || Y == GridHeight - 1;
			if (bIsBorder)
            Grid.SetCell(FIntPoint(X, Y), ESnakeCellType::Wall);
		}
	}
	
	FIntPoint HeadCell = WorldToGrid(GetActorLocation());
	Grid.SetCell(HeadCell, ESnakeCellType::Snake);
	
	//Initializing snake body
	SetupSegmentPositions();
	
	// Event trigger for food collection
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ASnakePawn::OnOverlapBegin);
	
	// Event trigger for collision (walls)
	CollisionComponent->OnComponentHit.AddDynamic(this, &ASnakePawn::OnHit);

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* InputLocalPlayerSubsystem =
				LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (InputMappingContext)
				{
					InputLocalPlayerSubsystem->AddMappingContext(InputMappingContext, 0);
				}
			}
		}
	}
	
	// Spawn and display the HUD
	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<USnakeHUD>(GetWorld(), HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
			HUDWidget->UpdateScore(0); // Show "Score: 0" at start
		}
	}
}

// Called every frame
void ASnakePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	UpdateBoostState(DeltaTime);
	UpdateMovement(DeltaTime);
	UpdateHUDBoost();
	UpdateSegments();
	UpdateGrid();
}

// --------------------------------------------------------------------------------------------------------
// INPUT
// --------------------------------------------------------------------------------------------------------
#pragma region INPUT 

// Called to bind functionality to input
void ASnakePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Binds actions
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASnakePawn::Move);
        EnhancedInput->BindAction(MoveAction, ETriggerEvent::Completed, this, &ASnakePawn::Move);
        EnhancedInput->BindAction(TurnAction, ETriggerEvent::Triggered, this, &ASnakePawn::Turn);
        EnhancedInput->BindAction(TurnAction, ETriggerEvent::Completed, this, &ASnakePawn::Turn);
		EnhancedInput->BindAction(BoostAction, ETriggerEvent::Triggered, this, &ASnakePawn::OnBoostPressed);
		EnhancedInput->BindAction(BoostAction, ETriggerEvent::Completed, this, &ASnakePawn::OnBoostReleased);
    }
}

void ASnakePawn::Move(const FInputActionValue& Value)
{
	MoveInput = Value.Get<float>(); 
}
void ASnakePawn::Turn(const FInputActionValue& Value)
{
	TurnInput = Value.Get<float>();
}

void ASnakePawn::OnBoostPressed()
{
	bWantsToBoost = true;
}
void ASnakePawn::OnBoostReleased()
{
	bWantsToBoost = false;
}

#pragma endregion


// --------------------------------------------------------------------------------------------------------
// SEGMENTS 
// --------------------------------------------------------------------------------------------------------
#pragma region SEGMENTS || Snake position & segments
void ASnakePawn::SetupSegmentPositions()
{
    // 1. Pre-fill position history with positions behind the head
    //    so segment placement has valid indices from the start.
    const FVector HeadLocation = GetActorLocation();
    const FVector BackwardDir = -GetActorForwardVector();

    // We need at least (InitialSegments * SegmentSpacing) entries in history
    int32 RequiredHistory = (InitialSegments + 1) * SegmentSpacing;
    PositionHistory.Empty();
    PositionHistory.Reserve(RequiredHistory);

    for (int32 i = 0; i < RequiredHistory; i++)
    {
        // Each slot is 1 unit apart; segments sample every SegmentSpacing slots
        PositionHistory.Add(HeadLocation + BackwardDir * i);
    }

    // 2. Spawn segments and place them at their correct history offset
    for (int32 i = 0; i < InitialSegments; i++)
    {
        AddSegment(); // spawns and appends to Segments[]

        int32 HistoryIndex = (i + 1) * SegmentSpacing;
        Segments[i]->SetActorLocation(PositionHistory[HistoryIndex]);
    }
}

void ASnakePawn::AddSegment()
{
    if (!SegmentClass) return;

    FVector SpawnLocation = PositionHistory.IsValidIndex(Segments.Num() + 1 * SegmentSpacing)
        ? PositionHistory[Segments.Num() * SegmentSpacing]
        : GetActorLocation();

    ASnakeSegment* NewSegment = GetWorld()->SpawnActor<ASnakeSegment>(
        SegmentClass, SpawnLocation, FRotator::ZeroRotator);

    if (NewSegment)
    {
    	AActor* AttachTarget = Segments.Num() > 0
        ? Cast<AActor>(Segments.Last())  // Last segment (before adding new one)
        : Cast<AActor>(this);            // Parent (the snake head)
    	
    	NewSegment->AttachToActor(AttachTarget, FAttachmentTransformRules::KeepWorldTransform);
    	Segments.Add(NewSegment);
    	NewSegment->SetMaterial(GetNextMaterial());
    }
}

#pragma endregion


// --------------------------------------------------------------------------------------------------------
// COLLISIONS 
// --------------------------------------------------------------------------------------------------------
#pragma region COLLISION || Overlap & OnHit
// Food collecting function
void ASnakePawn::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
	if (AFood* Food = Cast<AFood>(OtherActor))
	{
		const FFoodData& Data = Food->FoodData;
		
		Score += Data.PointValue; // Adding the score equivalent to the food type's points 
		Food->OnEaten(); // Calls the Food class method OnEaten 
		AddSegment(); 
		UpdateHUDScore(); 
		
		// Apply powers based on food type
		switch (Data.FoodType)
		{
			case EFoodType::SpeedBoost:
			ApplyUnlimitedBoost(Data.BenefitDuration); 
			break;
			
		case EFoodType::Golden:
			ApplyInvisibility(Data.BenefitDuration);
			break;
			
		default: break; // Basic and Special have no powers
		}
	}
}

// Collision function
void ASnakePawn::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Log every hit regardless of tag
	UE_LOG(LogTemp, Warning, TEXT("OnHit fired! Hit actor: %s"), *OtherActor->GetName());
	
	if (bIsInvisible) return; // Golden power active, ignore walls

    if (OtherActor && OtherActor->ActorHasTag(FName("Wall")))
    {
        if (ASnakeGameMode* GameMode = Cast<ASnakeGameMode>(GetWorld()->GetAuthGameMode()))
        {
        	UE_LOG(LogTemp, Warning, TEXT("Hit a WALL!"));
            GameMode->OnGameOver();       
        	Destroy();
        }
    }
}
#pragma endregion 


// --------------------------------------------------------------------------------------------------------
// HELPER FUNCTIONS 
// --------------------------------------------------------------------------------------------------------
#pragma region HELPERS || Movement, Boost, Segments, Grid, Hud, Material
void ASnakePawn::UpdateBoostState(float DeltaTime)
{
	if (bUnlimitedBoost)
	{
		BoostCharge = 1.0f;
	}
	else if (bWantsToBoost && BoostCharge > 0.0f)
	{
		BoostCharge = FMath::Clamp(BoostCharge - BoostDrainRate * DeltaTime, 0.0f, 1.0f);
	}
	else
	{
		BoostCharge = FMath::Clamp(BoostCharge + BoostRefillRate * DeltaTime, 0.0f, 1.0f);
	}
}

void ASnakePawn::UpdateMovement(float DeltaTime)
{
	// Rotation
	if (!FMath::IsNearlyZero(TurnInput))
	{
		AddActorLocalRotation(FRotator(0.0f, TurnInput * TurnSpeed * DeltaTime, 0.0f));
	}

	// Forward movement — speed depends on current boost state
	const bool bIsBoosting = bUnlimitedBoost || (bWantsToBoost && BoostCharge > 0.0f);
	const float CurrentSpeed = MoveSpeed * ActiveSpeedMultiplier * (bIsBoosting ? BoostSpeedMultiplier : 1.0f);
	AddActorWorldOffset(GetActorForwardVector() * CurrentSpeed * DeltaTime, true);
}

void ASnakePawn::UpdateSegments() 
{
	PositionHistory.Insert(GetActorLocation(), 0);    // Save head position to history
	
	for (int32 i = 0; i < Segments.Num(); i++)					 // Move each segment to its position in history
	{
		int32 HistoryIndex = (i + 1) * SegmentSpacing;
		if (PositionHistory.IsValidIndex(HistoryIndex))
		{
			Segments[i]->SetActorLocation(PositionHistory[HistoryIndex]);
		}
	}
}

void ASnakePawn::UpdateGrid()
{
	Grid.Clear(); 
	Grid.SetCell(WorldToGrid(GetActorLocation()), ESnakeCellType::Snake);
	for (ASnakeSegment* Seg : Segments)
	{
		Grid.SetCell(WorldToGrid(Seg->GetActorLocation()), ESnakeCellType::Snake);
	}
}

void ASnakePawn::UpdateHUDScore() const
{
    if (HUDWidget)
    {
        HUDWidget->UpdateScore(Score);
    }
}

void ASnakePawn::UpdateHUDBoost()
{
    if (HUDWidget)
    {
        HUDWidget->UpdateBoost(BoostCharge, bUnlimitedBoost);
    }
}

UMaterialInstance* ASnakePawn::GetNextMaterial()
{
	if (WatercolorMaterials.IsEmpty()) return nullptr;

	int32 Index;
	do
	{
		Index = FMath::RandRange(0, WatercolorMaterials.Num() - 1);
	}
	while (Index == LastMaterialIndex && WatercolorMaterials.Num() > 1);

	LastMaterialIndex = Index;
	return WatercolorMaterials[Index];
}
#pragma endregion


// --------------------------------------------------------------------------------------------------------
// HELPER FUNCTIONS 
// --------------------------------------------------------------------------------------------------------
#pragma region GRID
// Take the world space and make it into a grid 
FIntPoint ASnakePawn::WorldToGrid(FVector WorldPos) const
{
    return FIntPoint(
        FMath::FloorToInt(WorldPos.X / CellSize),
        FMath::FloorToInt(WorldPos.Y / CellSize)
    );
}

// Multiply back so every cell centers inside the cell (otherwise it will snap to the corner)
FVector ASnakePawn::GridToWorld(FIntPoint GridPos) const
{
    return FVector(
        GridPos.X * CellSize + CellSize * 0.5f,
        GridPos.Y * CellSize + CellSize * 0.5f,
        GetActorLocation().Z
    );
}

FVector ASnakePawn::GetRandomEmptyCell() const
{
    TArray<FIntPoint> EmptyCells;
    for (int32 Y = 0; Y < GridHeight; Y++)
        for (int32 X = 0; X < GridWidth; X++)
        {
            FIntPoint Pos(X, Y);
            if (Grid.GetCell(Pos) == ESnakeCellType::Empty)
                EmptyCells.Add(Pos);
        }

    if (EmptyCells.IsEmpty()) return FVector::ZeroVector;
    
    FIntPoint Chosen = EmptyCells[FMath::RandRange(0, EmptyCells.Num() - 1)];
    return GridToWorld(Chosen);
}
#pragma endregion


// --------------------------------------------------------------------------------------------------------
// SNAKE POWER-UPS
// --------------------------------------------------------------------------------------------------------
#pragma region SNAKE POWER-UPS || SpeedBoost, Invisibility
void ASnakePawn::ApplySpeedBoost(float Multiplier, float Duration)
{
    // Clear existing timer if already active
    GetWorldTimerManager().ClearTimer(SpeedBoostTimer);
    
    ActiveSpeedMultiplier = Multiplier;
    
    GetWorldTimerManager().SetTimer(
        SpeedBoostTimer,
        this,
        &ASnakePawn::RemoveSpeedBoost,
        Duration,
        false  // Don't loop
    );
}

void ASnakePawn::RemoveSpeedBoost()
{
    ActiveSpeedMultiplier = 1.0f;
}

void ASnakePawn::ApplyInvisibility(float Duration)
{
    GetWorldTimerManager().ClearTimer(InvisibilityTimer);
    
    bIsInvisible = true;
    
    GetWorldTimerManager().SetTimer(
        InvisibilityTimer,
        this,
        &ASnakePawn::RemoveInvisibility,
        Duration,
        false
    );
}

void ASnakePawn::RemoveInvisibility()
{
    bIsInvisible = false;
}

void ASnakePawn::ApplyUnlimitedBoost(float Duration)
{
    GetWorldTimerManager().ClearTimer(UnlimitedBoostTimer);
    bUnlimitedBoost = true;
    GetWorldTimerManager().SetTimer(
        UnlimitedBoostTimer, this,
        &ASnakePawn::RemoveUnlimitedBoost,
        Duration, false);
}

void ASnakePawn::RemoveUnlimitedBoost()
{
    bUnlimitedBoost = false;
}

#pragma endregion