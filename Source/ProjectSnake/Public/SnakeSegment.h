#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SnakeSegment.generated.h"

UCLASS()
class PROJECTSNAKE_API ASnakeSegment : public AActor
{
    GENERATED_BODY()
    
public: 
    ASnakeSegment();
    void SetMaterial(UMaterialInterface* Material);

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<UStaticMeshComponent> VisualMesh;
};