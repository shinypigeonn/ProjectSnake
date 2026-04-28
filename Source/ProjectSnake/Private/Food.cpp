// Fill out your copyright notice in the Description page of Project Settings.

#include "Food.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AFood::AFood()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	FoodMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FoodMesh"));
	RootComponent = FoodMesh;
	
	// Collision enabled and generates overlap events.
	FoodMesh->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	FoodMesh->SetGenerateOverlapEvents(true);
}

void AFood::BeginPlay()
{
	Super::BeginPlay();
}

void AFood::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// TODO: Food timer for only Golden? 
	
	// If timelimit is bigger than 0 = count to the time
	// limit, then if time elapse is greater/equal to time limit, 
	// then destory food actor
	// if food is SpeedBoost OR Golden then do timer, other fruits will not have time limit
	/*if (TimeLimit > 0.0f)
	{
		TimeElapsed += DeltaTime;
		if (TimeElapsed >= TimeLimit)
		{
			// Time ran out, actor gets destroyed
			Destroy();
		}
	}*/
}

void AFood::OnEaten()
{
	// GameManager should call GetPointValue() then pass it to ScoreManager.
	// Override in Blueprint children for any special on-eaten behavior
	// (e.g. speed boost, score multiplier, poison penalty).
	SoundEffect();
	VisualEffect();
	Destroy();
}
