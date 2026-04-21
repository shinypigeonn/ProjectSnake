// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GridMapHUD.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTSNAKE_API UGridMapHUD : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateGrid();
};
