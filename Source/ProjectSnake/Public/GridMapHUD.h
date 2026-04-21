// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SnakeGrid.h"
#include "GridMapHUD.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTSNAKE_API UGridMapHUD : public UUserWidget
{
	GENERATED_BODY()
	
public:
    // Called from SnakePawn to give the widget grid data
    UFUNCTION(BlueprintCallable)
    void SetGridData(int32 InWidth, int32 InHeight, const TArray<ESnakeCellType>& InCells);

    // Grid data stored locally for drawing
    UPROPERTY(BlueprintReadOnly)
    int32 GridWidth = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 GridHeight = 0;

    UPROPERTY(BlueprintReadOnly)
    TArray<ESnakeCellType> GridCells;
	
	 // Configurable colors — tweak in Blueprint Class Defaults
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Map Colors")
    FLinearColor EmptyColor = FLinearColor(0.1f, 0.1f, 0.1f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Map Colors")
    FLinearColor SnakeColor = FLinearColor::Green;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Map Colors")
    FLinearColor FoodColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Map Colors")
    FLinearColor WallColor = FLinearColor::Gray;

protected:
    virtual int32 NativePaint(
        const FPaintArgs& Args,
        const FGeometry& AllottedGeometry,
        const FSlateRect& MyCullingRect,
        FSlateWindowElementList& OutDrawElements,
        int32 LayerId,
        const FWidgetStyle& InWidgetStyle,
        bool bParentEnabled) const override;
};
