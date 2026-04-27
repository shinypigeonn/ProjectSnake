// Fill out your copyright notice in the Description page of Project Settings

#include "SnakePawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "Food.h"
#include "MultiplayerSnakeMode.h"

// ────────────────────────────────────────────────────────────────────────────────
// CONSTRUCTOR
// ────────────────────────────────────────────────────────────────────────────────

ASnakePawn::ASnakePawn()
{
	PrimaryActorTick.bCanEverTick = true;
	
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	SetRootComponent(CollisionComponent);
	CollisionComponent->InitSphereRadius(50.0f);
	
	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(CollisionComponent);
	
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(CollisionComponent);
	SpringArm->TargetArmLength = 700.0f;
	SpringArm->SetRelativeRotation(FRotator(-55.0f, 0.0f, 0.0f));
	SpringArm->bDoCollisionTest = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = false;
	SpringArm->bInheritYaw = false;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArm);
}

// ────────────────────────────────────────────────────────────────────────────────
// BEGIN PLAY
// ────────────────────────────────────────────────────────────────────────────────

void ASnakePawn::BeginPlay()
{
	Super::BeginPlay();
	InitGrid();
	InitSnake();
	InitHUD(); // Safe here — controller is valid by BeginPlay when possessed by GameMode
}

// ────────────────────────────────────────────────────────────────────────────────
// TICK
// ────────────────────────────────────────────────────────────────────────────────

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
// INPUT — called by SnakePlayerController
// ────────────────────────────────────────────────────────────────────────────────

void ASnakePawn::Move(const FInputActionValue& Value)
{
	// Currently move forward is always-on; this can be used for speed modulation
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
// COLLISION
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
		case EFoodType::SpeedBoost: ApplyUnlimitedBoost(Data.BenefitDuration); break;
		case EFoodType::Golden:     ApplyInvisibility(Data.BenefitDuration);   break;
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

        if (AMultiplayerSnakeMode* GameMode = Cast<AMultiplayerSnakeMode>(GetWorld()->GetAuthGameMode()))
        {
            UE_LOG(LogTemp, Warning, TEXT("Hit a snake segment!"));
            GameMode->OnPlayerDied(this);
        }
	}
}

void ASnakePawn::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	UE_LOG(LogTemp, Warning, TEXT("OnHit: %s"), *OtherActor->GetName());
	
	if (bIsInvisible) return;

    if (OtherActor && OtherActor->ActorHasTag(FName("Wall")))
    {
        if (AMultiplayerSnakeMode* GameMode = Cast<AMultiplayerSnakeMode>(GetWorld()->GetAuthGameMode()))
        {
        	UE_LOG(LogTemp, Warning, TEXT("Hit a WALL!"));
            GameMode->OnPlayerDied(this);
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
		BoostCharge = 1.0f;
	else if (bWantsToBoost && BoostCharge > 0.0f)
		BoostCharge = FMath::Clamp(BoostCharge - BoostDrainRate * DeltaTime, 0.0f, 1.0f);
	else
		BoostCharge = FMath::Clamp(BoostCharge + BoostRefillRate * DeltaTime, 0.0f, 1.0f);
}

void ASnakePawn::UpdateMovement(float DeltaTime)
{
	if (!FMath::IsNearlyZero(TurnInput))
		AddActorLocalRotation(FRotator(0.0f, TurnInput * TurnSpeed * DeltaTime, 0.0f));

	const bool  bIsBoosting  = bUnlimitedBoost || (bWantsToBoost && BoostCharge > 0.0f);
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
    if (HUDWidget) HUDWidget->UpdateScore(Score);
}

void ASnakePawn::UpdateHUDBoost() const
{
    if (HUDWidget) HUDWidget->UpdateBoost(BoostCharge, bUnlimitedBoost);
}

UMaterialInstance* ASnakePawn::GetNextMaterial()
{
	if (WatercolorMaterials.IsEmpty()) return nullptr;
	int32 Index;
	do { Index = FMath::RandRange(0, WatercolorMaterials.Num() - 1); }
	while (Index == LastMaterialIndex && WatercolorMaterials.Num() > 1);
	LastMaterialIndex = Index;
	return WatercolorMaterials[Index];
}

void ASnakePawn::InitGrid()
{
	Grid = FSnakeGrid(GridWidth, GridHeight);
	for (int32 X = 0; X < GridWidth; X++)
		for (int32 Y = 0; Y < GridHeight; Y++)
		{
			bool bIsBorder = X == 0 || X == GridWidth - 1 || Y == 0 || Y == GridHeight - 1;
			if (bIsBorder) Grid.SetCell(FIntPoint(X, Y), ESnakeCellType::Wall);
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
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;
	HUDWidget = CreateWidget<USnakeHUD>(PC, HUDWidgetClass);
	if (HUDWidget)
	{
		HUDWidget->AddToPlayerScreen();
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
        FMath::FloorToInt(WorldPos.Y / CellSize));
}

FVector ASnakePawn::GridToWorld(FIntPoint GridPos) const
{
    return FVector(
        GridPos.X * CellSize + CellSize * 0.5f,
        GridPos.Y * CellSize + CellSize * 0.5f,
        GetActorLocation().Z);
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
    return GridToWorld(EmptyCells[FMath::RandRange(0, EmptyCells.Num() - 1)]);
}

#pragma endregion

// ────────────────────────────────────────────────────────────────────────────────
// POWER-UPS
// ────────────────────────────────────────────────────────────────────────────────
#pragma region POWER-UPS

void ASnakePawn::ApplySpeedBoost(float Multiplier, float Duration)
{
    GetWorldTimerManager().ClearTimer(SpeedBoostTimer);
    ActiveSpeedMultiplier = Multiplier;
    GetWorldTimerManager().SetTimer(SpeedBoostTimer, this, &ASnakePawn::RemoveSpeedBoost, Duration, false);
}
void ASnakePawn::RemoveSpeedBoost() { ActiveSpeedMultiplier = 1.0f; }

void ASnakePawn::ApplyInvisibility(float Duration)
{
    GetWorldTimerManager().ClearTimer(InvisibilityTimer);
    bIsInvisible = true;
    GetWorldTimerManager().SetTimer(InvisibilityTimer, this, &ASnakePawn::RemoveInvisibility, Duration, false);
}
void ASnakePawn::RemoveInvisibility() { bIsInvisible = false; }

void ASnakePawn::ApplyUnlimitedBoost(float Duration)
{
    GetWorldTimerManager().ClearTimer(UnlimitedBoostTimer);
    bUnlimitedBoost = true;
    GetWorldTimerManager().SetTimer(UnlimitedBoostTimer, this, &ASnakePawn::RemoveUnlimitedBoost, Duration, false);
}
void ASnakePawn::RemoveUnlimitedBoost() { bUnlimitedBoost = false; }

#pragma endregion