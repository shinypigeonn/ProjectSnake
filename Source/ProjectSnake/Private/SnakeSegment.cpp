#include "SnakeSegment.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

ASnakeSegment::ASnakeSegment()
{
    PrimaryActorTick.bCanEverTick = false; // Segments don't need tick

    VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
    SetRootComponent(VisualMesh);
    VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); 
    
    OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
    OverlapSphere->SetupAttachment(VisualMesh);
    OverlapSphere->InitSphereRadius(30.0f);
    OverlapSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    OverlapSphere->SetGenerateOverlapEvents(true);
    
    Tags.Add(TEXT("SnakeSegment"));
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