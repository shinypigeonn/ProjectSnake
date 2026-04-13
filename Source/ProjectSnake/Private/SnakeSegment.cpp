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
void ASnakeSegment::SetMaterial(UMaterialInterface* Material)
{
    if (VisualMesh && Material)
    {
        VisualMesh->SetMaterial(0, Material);
    }
}