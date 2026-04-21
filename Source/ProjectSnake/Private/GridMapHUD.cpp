// Fill out your copyright notice in the Description page of Project Settings.


#include "GridMapHUD.h"
#include "Rendering/DrawElements.h"

void UGridMapHUD::SetGridData(int32 InWidth, int32 InHeight, const TArray<ESnakeCellType>& InCells)
{
    GridWidth = InWidth;
    GridHeight = InHeight;
    GridCells = InCells;
    InvalidateLayoutAndVolatility();
}

int32 UGridMapHUD::NativePaint(
    const FPaintArgs& Args,
    const FGeometry& AllottedGeometry,
    const FSlateRect& MyCullingRect,
    FSlateWindowElementList& OutDrawElements,
    int32 LayerId,
    const FWidgetStyle& InWidgetStyle,
    bool bParentEnabled) const
{
    int32 Layer = Super::NativePaint(Args, AllottedGeometry, MyCullingRect,
        OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

    if (GridWidth == 0 || GridHeight == 0) return Layer;

    FVector2D MapSize = AllottedGeometry.GetLocalSize();
    float CellW = MapSize.X / GridWidth;
    float CellH = MapSize.Y / GridHeight;

    for (int32 Y = 0; Y < GridHeight; Y++)
    {
        for (int32 X = 0; X < GridWidth; X++)
        {
            int32 Index = Y * GridWidth + X;
            if (!GridCells.IsValidIndex(Index)) continue;

            FLinearColor Color;
            switch (GridCells[Index])
            {
                case ESnakeCellType::Snake: Color = SnakeColor; break;
                case ESnakeCellType::Food:  Color = FoodColor;  break;
                case ESnakeCellType::Wall:  Color = WallColor;  break;
                default:                   Color = EmptyColor;  break;
            }

            FSlateDrawElement::MakeBox(
                OutDrawElements,
                Layer,
                AllottedGeometry.ToPaintGeometry(
                    FVector2D(X * CellW, Y * CellH),
                    FVector2D(CellW - 1.0f, CellH - 1.0f)), // -1 gives grid lines
                FCoreStyle::Get().GetBrush("WhiteBrush"),
                ESlateDrawEffect::None,
                Color
            );
        }
    }
    return Layer;
}