#pragma once

#include "CoreMinimal.h"
#include "FoodData.generated.h"

UENUM(BlueprintType)
enum class EFoodType : uint8
{
    Basic       UMETA(DisplayName="Basic"), // Least number of points (common fruits)
    Special     UMETA(DisplayName="Special"), // More points (not as common fruits)
    SpeedBoost  UMETA(DisplayName="Speed Boost"), // Food that gives speed boost
    Golden      UMETA(DisplayName="Golden") // Food that gives invisibility
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