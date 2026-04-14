#pragma once

#include "CoreMinimal.h"
#include "FoodData.generated.h"

UENUM(BlueprintType)
enum class EFoodType : uint8
{
    Basic       UMETA(DisplayName="Basic"),
    Special     UMETA(DisplayName="Special"),
    SpeedBoost  UMETA(DisplayName="Speed Boost"),
    Golden      UMETA(DisplayName="Golden")
};

USTRUCT(BlueprintType)
struct FFoodData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EFoodType FoodType = EFoodType::Basic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PointValue = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BenefitDuration = 0.0f;
};