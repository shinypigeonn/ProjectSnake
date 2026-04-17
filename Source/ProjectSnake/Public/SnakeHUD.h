// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SnakeHUD.generated.h"

UCLASS()
class PROJECTSNAKE_API USnakeHUD : public UUserWidget
{
	GENERATED_BODY()
	
public: 
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ScoreText;
	
	UFUNCTION(BlueprintCallable, Category="HUD")
	void UpdateScore(int32 NewScore);
	
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateBoost(float Charge, bool bIsUnlimited);

};
