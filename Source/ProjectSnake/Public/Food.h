// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FoodData.h"
#include "Food.generated.h"

UCLASS()
class PROJECTSNAKE_API AFood : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFood();	
	
	// TODO: Make "Foodpoints" to a blueprint function that uses enum, 
	// "BASIC" "SPECIAL" "SPEED BOOST" "GOLDEN(INVISIBILITY), t
	// hat function controls the data of the fruit (target selector?), and its points, and "benefits"
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FFoodData FoodData;
	
	UFUNCTION(BlueprintImplementableEvent)
	void VisualEffect();
	
	UFUNCTION(BlueprintImplementableEvent)
	void SoundEffect();
	
	virtual void Tick(float DeltaTime) override;
	
	// Called when the snake eats this food
	// Override in Blueprint children for special on-eaten behavior
	UFUNCTION(BlueprintCallable, Category="Food")
	void OnEaten();
	
protected:
	virtual void BeginPlay() override;
	
	// The mesh that represents the food visually
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> FoodMesh;
	
	// How long this food stays spawned before disappearing (0 = no limit)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Food")
	float TimeLimit = 1.0f;

private:
	// Tracks how long this food has been alive
	float TimeElapsed = 0.0f;
};
