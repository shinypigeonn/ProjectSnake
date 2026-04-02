// Fill out your copyright notice in the Description page of Project Settings.

#include "SnakeHUD.h"
#include "Components/TextBlock.h"

void USnakeHUD::UpdateScore(int32 NewScore)
{
	if (ScoreText)
	{
		ScoreText->SetText(
		FText::FromString(FString::Printf(TEXT("SCORE: %d"), NewScore)));
	};
}
