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

	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

// Called when the game starts or when spawned
void ASnakePawn::BeginPlay()
{
	// Runtime intialization, adding input mapping contexts, reference that depend on having a world
	Super::BeginPlay();
	
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

    // Save head position to history
    PositionHistory.Insert(GetActorLocation(), 0);
	
    // Move each segment to its position in history
    for (int32 i = 0; i < Segments.Num(); i++)
    {
        int32 HistoryIndex = (i + 1) * SegmentSpacing;
        if (PositionHistory.IsValidIndex(HistoryIndex))
        {
            Segments[i]->SetActorLocation(PositionHistory[HistoryIndex]);
        }
    }

    if (!FMath::IsNearlyZero(TurnInput))
    {
        AddActorLocalRotation(FRotator(0.0f, TurnInput * TurnSpeed * DeltaTime, 0.0f));
    }
	const FVector Delta = GetActorForwardVector() * MoveSpeed * DeltaTime;
	AddActorWorldOffset(Delta, true);
}

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

void ASnakePawn::SetupSegmentPositions()
{
    // 1. Pre-fill position history with positions behind the head
    //    so segment placement has valid indices from the start.
    FVector HeadLocation = GetActorLocation();
    FVector BackwardDir = -GetActorForwardVector();

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

void ASnakePawn::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
	if (AFood* Food = Cast<AFood>(OtherActor))
	{
		Score += Food->FoodData.PointValue;
		Food->OnEaten();
		AddSegment();
		UpdateHUDScore();
	}
}

// Collision function
void ASnakePawn::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Log every hit regardless of tag
	UE_LOG(LogTemp, Warning, TEXT("OnHit fired! Hit actor: %s"), *OtherActor->GetName());

    if (OtherActor && OtherActor->ActorHasTag(FName("Wall")))
    {
        if (ASnakeGameMode* GameMode = Cast<ASnakeGameMode>(GetWorld()->GetAuthGameMode()))
        {
        	UE_LOG(LogTemp, Warning, TEXT("Hit a WALL!"));
            GameMode->OnGameOver();
        }
        Destroy();
    }
}

void ASnakePawn::UpdateHUDScore()
{
    if (HUDWidget)
    {
        HUDWidget->UpdateScore(Score);
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