#include "SnakeSegment.h"
#include "Components/StaticMeshComponent.h"

ASnakeSegment::ASnakeSegment()
{
    PrimaryActorTick.bCanEverTick = false; // Segments don't need tick

    VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
    SetRootComponent(VisualMesh);
}

void ASnakeSegment::BeginPlay()
{
    Super::BeginPlay();
}
