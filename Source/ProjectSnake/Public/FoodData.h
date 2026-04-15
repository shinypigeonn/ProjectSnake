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

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EFoodType FoodType = EFoodType::Basic;

    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
    int32 PointValue = 10;
    
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
    float SpeedMultiplier = 1.0f;

    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
    float BenefitDuration = 0.0f;
    
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
    bool bGrantsInvisibility = false;


    static FFoodData Basic()
    {
        FFoodData Data;
        Data.FoodType = EFoodType::Basic;
        Data.PointValue = 10;
        Data.SpeedMultiplier = 1.0f;
        Data.BenefitDuration = 0.0f;
        Data.bGrantsInvisibility = false;
        return Data;
    }

    static FFoodData Special()
    {
        FFoodData Data;
        Data.FoodType = EFoodType::Special;
        Data.PointValue = 30;
        Data.SpeedMultiplier = 1.0f;
        Data.BenefitDuration = 0.0f;
        Data.bGrantsInvisibility = false;
        return Data;
    }

    static FFoodData SpeedBoost()
    {
        FFoodData Data;
        Data.FoodType = EFoodType::SpeedBoost;
        Data.PointValue = 30;
        Data.SpeedMultiplier = 1.5f;
        Data.BenefitDuration = 0.0f;
        Data.bGrantsInvisibility = false;
        return Data;
    }

    static FFoodData Golden()
    {
        FFoodData Data;
        Data.FoodType = EFoodType::Golden;
        Data.PointValue = 100;
        Data.SpeedMultiplier = 1.0f;
        Data.BenefitDuration = 0.0f;
        Data.bGrantsInvisibility = true;
        return Data;
    }
};