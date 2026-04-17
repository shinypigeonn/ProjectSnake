// SnakeGrid.h
#pragma once
#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class ESnakeCellType : uint8
{
    Empty   UMETA(DisplayName = "Empty"),
    Snake   UMETA(DisplayName = "Snake"),
    Food    UMETA(DisplayName = "Food"),
    Wall    UMETA(DisplayName = "Wall")
};

struct FSnakeGrid
{
    int32 Width  = 0;
    int32 Height = 0;

    TArray<ESnakeCellType> Cells;

    FSnakeGrid() = default;
    FSnakeGrid(int32 InWidth, int32 InHeight)
        : Width(InWidth), Height(InHeight)
    {
        Cells.Init(ESnakeCellType::Empty, Width * Height);
    }

    bool InBounds(FIntPoint Pos) const
    {
        return Pos.X >= 0 && Pos.X < Width
            && Pos.Y >= 0 && Pos.Y < Height;
    }

    int32 ToIndex(FIntPoint Pos) const
    {
        return Pos.Y * Width + Pos.X;
    }

    void SetCell(FIntPoint Pos, ESnakeCellType Value)
    {
        if (InBounds(Pos))
            Cells[ToIndex(Pos)] = Value;
    }

    ESnakeCellType GetCell(FIntPoint Pos) const
    {
        if (InBounds(Pos))
            return Cells[ToIndex(Pos)];
        return ESnakeCellType::Empty;
    }

    void Clear()
    {
        Cells.Init(ESnakeCellType::Empty, Width * Height);
    }

    FIntPoint WrapPosition(FIntPoint Pos) const
    {
        if (Pos.X < 0)       Pos.X = Width - 1;
        if (Pos.X >= Width)  Pos.X = 0;
        if (Pos.Y < 0)       Pos.Y = Height - 1;
        if (Pos.Y >= Height) Pos.Y = 0;
        return Pos;
    }
};