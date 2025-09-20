// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "EAttackPowerType.h"
#include "Engine/DataTable.h"
#include "FPadFeedbackData.generated.h"

USTRUCT(BlueprintType)
struct FPadFeedbackData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Amplitude = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.f;
};
 
static FPadFeedbackData GetPadFeedbackData(EAttackPowerType Type)
{
	switch (Type)
	{
	case EAttackPowerType::Small:   return {0.2f, 0.2f};
	case EAttackPowerType::Normal:  return {0.4f, 0.3f};
	case EAttackPowerType::Large:   return {0.7f, 0.45f};
	case EAttackPowerType::Huge:    return {1.0f, 0.6f};
	default:                        return {};
	}
}