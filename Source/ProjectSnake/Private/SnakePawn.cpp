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
	PrimaryActorTick.bCanEverTick = true;
	
	// --- Collision ---
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	SetRootComponent(CollisionComponent);
	CollisionComponent->InitSphereRadius(50.0f);
	
	// --- Mesh ---
	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(CollisionComponent);
	
	// --- SpringArm ---
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(CollisionComponent);
	SpringArm->TargetArmLength = 700.0f;
	SpringArm->SetRelativeRotation(FRotator(-55.0f, 0.0f, 0.0f));
	SpringArm->bDoCollisionTest = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = false;
	SpringArm->bInheritYaw = false;

	// --- Camera ---
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArm);

	// GameMode handles possession — do not auto-possess
	AutoPossessPlayer = EAutoReceiveInput::Disabled;
}

// Called when the game starts or when spawned
void ASnakePawn::BeginPlay()
{
	Super::BeginPlay();
	GameMode = Cast<ASnakeGameMode>(GetWorld()->GetAuthGameMode());
	
	InitGrid();
	InitSnake();
	InitHUD();
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


// ────────────────────────────────────────────────────────────────────────────────
// INPUT
// ────────────────────────────────────────────────────────────────────────────────
#pragma region INPUT

void ASnakePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInput->BindAction(IA_Move,  ETriggerEvent::Triggered, this, &ASnakePawn::Move);
		EnhancedInput->BindAction(IA_Move,  ETriggerEvent::Completed, this, &ASnakePawn::Move);
		EnhancedInput->BindAction(IA_Turn,  ETriggerEvent::Triggered, this, &ASnakePawn::Turn);
		EnhancedInput->BindAction(IA_Turn,  ETriggerEvent::Completed, this, &ASnakePawn::Turn);
		EnhancedInput->BindAction(IA_Boost, ETriggerEvent::Triggered, this, &ASnakePawn::OnBoostPressed);
		EnhancedInput->BindAction(IA_Boost, ETriggerEvent::Completed, this, &ASnakePawn::OnBoostReleased);
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

// Store the IMC the GameMode wants this pawn to use.
// Must be called before Possess() so it's ready when PawnClientRestart fires.
void ASnakePawn::ApplyIMC(UInputMappingContext* IMC)
{
	AssignedIMC = IMC;
}

// Fires after the controller fully possesses this pawn
void ASnakePawn::PawnClientRestart()
{
	Super::PawnClientRestart();

	// Prefer the IMC injected by GameMode; fall back to the Blueprint default
	UInputMappingContext* IMCToUse = AssignedIMC ? AssignedIMC : InputMappingContext;

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
			if (auto* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
				if (IMCToUse)
					Subsystem->AddMappingContext(IMCToUse, 0);
}

#pragma endregion


// ────────────────────────────────────────────────────────────────────────────────
// SEGMENTS
// ────────────────────────────────────────────────────────────────────────────────
#pragma region SEGMENTS

void ASnakePawn::SetupSegmentPositions()
{
	const FVector HeadLocation = GetActorLocation();
	const FVector BackwardDir  = -GetActorForwardVector();

	int32 RequiredHistory = (InitialSegments + 1) * SegmentSpacing;
	PositionHistory.Empty();
	PositionHistory.Reserve(RequiredHistory);

	for (int32 i = 0; i < RequiredHistory; i++)
		PositionHistory.Add(HeadLocation + BackwardDir * i);

	for (int32 i = 0; i < InitialSegments; i++)
	{
		AddSegment();
		int32 HistoryIndex = (i + 1) * SegmentSpacing;
		Segments[i]->SetActorLocation(PositionHistory[HistoryIndex]);
	}
}

void ASnakePawn::AddSegment()
{
	if (!SegmentClass) return;

	int32 HistoryIndex = (Segments.Num() + 1) * SegmentSpacing;
	FVector SpawnLocation = PositionHistory.IsValidIndex(HistoryIndex)
		? PositionHistory[HistoryIndex]
		: GetActorLocation();

	ASnakeSegment* NewSegment = GetWorld()->SpawnActor<ASnakeSegment>(
		SegmentClass, SpawnLocation, FRotator::ZeroRotator);

	if (NewSegment)
	{
		Segments.Add(NewSegment);
		NewSegment->SetMaterial(GetNextMaterial());
	}
}

#pragma endregion


// ────────────────────────────────────────────────────────────────────────────────
// COLLISIONS
// ────────────────────────────────────────────────────────────────────────────────
#pragma region COLLISION

void ASnakePawn::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	// --- Food ---
	if (AFood* Food = Cast<AFood>(OtherActor))
	{
		const FFoodData& Data = Food->FoodData;
		
		Score += Data.PointValue;
		Food->OnEaten();
		AddSegment();
		UpdateHUDScore();
		
		switch (Data.FoodType)
		{
			case EFoodType::SpeedBoost:
				ApplyUnlimitedBoost(Data.BenefitDuration);
				break;
			case EFoodType::Golden:
				ApplyInvisibility(Data.BenefitDuration);
				break;
			default: break;
		}
	}
	
	// --- Snake segment collision ---
	if (OtherActor && OtherActor->ActorHasTag(FName("SnakeSegment")))
	{
		if (bIsInvisible) return;

		if (ASnakeSegment* HitSegment = Cast<ASnakeSegment>(OtherActor))
		{
			int32 SegIndex = Segments.IndexOfByKey(HitSegment);
			if (SegIndex >= 0 && SegIndex < 3) return; // ignore own near segments
		}
		
		if (GameMode)
		{
			UE_LOG(LogTemp, Warning, TEXT("Hit a snake segment — game over!"));
			GameMode->OnGameOver();
			Destroy();
		}
	}
}

void ASnakePawn::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	UE_LOG(LogTemp, Warning, TEXT("OnHit fired! Hit actor: %s"), *OtherActor->GetName());

	if (bIsInvisible) return;

	if (OtherActor && OtherActor->ActorHasTag(FName("Wall")))
	{
		if (GameMode)
		{
			UE_LOG(LogTemp, Warning, TEXT("Hit a WALL!"));
			GameMode->OnGameOver();
			Destroy();
		}
	}
}

#pragma endregion


// ────────────────────────────────────────────────────────────────────────────────
// HELPERS
// ────────────────────────────────────────────────────────────────────────────────
#pragma region HELPERS

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
	if (!GameMode || GameMode->GetCurrentState() != EGameState::Playing) return;;
	
	if (!FMath::IsNearlyZero(TurnInput))
		AddActorLocalRotation(FRotator(0.0f, TurnInput * TurnSpeed * DeltaTime, 0.0f));

	const bool bIsBoosting = bUnlimitedBoost || (bWantsToBoost && BoostCharge > 0.0f);
	const float CurrentSpeed = MoveSpeed * ActiveSpeedMultiplier * (bIsBoosting ? BoostSpeedMultiplier : 1.0f);
	AddActorWorldOffset(GetActorForwardVector() * CurrentSpeed * DeltaTime, true);
}

void ASnakePawn::UpdateSegments()
{
	PositionHistory.Insert(GetActorLocation(), 0);

	int32 MaxHistory = (Segments.Num() + 1) * SegmentSpacing + 1;
	if (PositionHistory.Num() > MaxHistory)
		PositionHistory.SetNum(MaxHistory);

	for (int32 i = 0; i < Segments.Num(); i++)
	{
		int32 HistoryIndex = (i + 1) * SegmentSpacing;
		if (PositionHistory.IsValidIndex(HistoryIndex))
			Segments[i]->SetActorLocation(PositionHistory[HistoryIndex]);
	}
}

void ASnakePawn::UpdateGrid()
{
	Grid.Clear();
	Grid.SetCell(WorldToGrid(GetActorLocation()), ESnakeCellType::Snake);
	for (ASnakeSegment* Seg : Segments)
		Grid.SetCell(WorldToGrid(Seg->GetActorLocation()), ESnakeCellType::Snake);
}

void ASnakePawn::UpdateHUDScore() const
{
	if (HUDWidget)
		HUDWidget->UpdateScore(Score);
}

void ASnakePawn::UpdateHUDBoost() const
{
	if (HUDWidget)
		HUDWidget->UpdateBoost(BoostCharge, bUnlimitedBoost);
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

// --- Initialization ---

void ASnakePawn::InitGrid()
{
	Grid = FSnakeGrid(GridWidth, GridHeight);

	for (int32 X = 0; X < GridWidth; X++)
		for (int32 Y = 0; Y < GridHeight; Y++)
		{
			bool bIsBorder = X == 0 || X == GridWidth - 1
						  || Y == 0 || Y == GridHeight - 1;
			if (bIsBorder)
				Grid.SetCell(FIntPoint(X, Y), ESnakeCellType::Wall);
		}

	Grid.SetCell(WorldToGrid(GetActorLocation()), ESnakeCellType::Snake);
}

void ASnakePawn::InitSnake()
{
	SetupSegmentPositions();
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ASnakePawn::OnOverlapBegin);
	CollisionComponent->OnComponentHit.AddDynamic(this, &ASnakePawn::OnHit);
}

void ASnakePawn::InitHUD()
{
	if (!HUDWidgetClass) return;

	HUDWidget = CreateWidget<USnakeHUD>(GetWorld(), HUDWidgetClass);
	if (HUDWidget)
	{
		HUDWidget->AddToViewport();
		HUDWidget->UpdateScore(0);
	}
}

#pragma endregion


// ────────────────────────────────────────────────────────────────────────────────
// GRID
// ────────────────────────────────────────────────────────────────────────────────
#pragma region GRID

FIntPoint ASnakePawn::WorldToGrid(FVector WorldPos) const
{
	return FIntPoint(
		FMath::FloorToInt(WorldPos.X / CellSize),
		FMath::FloorToInt(WorldPos.Y / CellSize)
	);
}

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


// ────────────────────────────────────────────────────────────────────────────────
// SNAKE POWER-UPS
// ────────────────────────────────────────────────────────────────────────────────
#pragma region SNAKE POWER-UPS

void ASnakePawn::ApplySpeedBoost(float Multiplier, float Duration)
{
	GetWorldTimerManager().ClearTimer(SpeedBoostTimer);
	ActiveSpeedMultiplier = Multiplier;
	GetWorldTimerManager().SetTimer(SpeedBoostTimer, this, &ASnakePawn::RemoveSpeedBoost, Duration, false);
}

void ASnakePawn::RemoveSpeedBoost()
{
	ActiveSpeedMultiplier = 1.0f;
}

void ASnakePawn::ApplyInvisibility(float Duration)
{
	GetWorldTimerManager().ClearTimer(InvisibilityTimer);
	bIsInvisible = true;
	GetWorldTimerManager().SetTimer(InvisibilityTimer, this, &ASnakePawn::RemoveInvisibility, Duration, false);
}

void ASnakePawn::RemoveInvisibility()
{
	bIsInvisible = false;
}

void ASnakePawn::ApplyUnlimitedBoost(float Duration)
{
	GetWorldTimerManager().ClearTimer(UnlimitedBoostTimer);
	bUnlimitedBoost = true;
	GetWorldTimerManager().SetTimer(UnlimitedBoostTimer, this, &ASnakePawn::RemoveUnlimitedBoost, Duration, false);
}

void ASnakePawn::RemoveUnlimitedBoost()
{
	bUnlimitedBoost = false;
}

#pragma endregion