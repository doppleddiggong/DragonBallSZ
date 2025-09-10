#pragma once

#include "CoreMinimal.h"
#include "EAttackPowerType.h"
#include "Engine/DataTable.h"
#include "FKnockbackData.generated.h"

USTRUCT(BlueprintType)
struct FKnockbackData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAttackPowerType Type = EAttackPowerType::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0"))
	float HorizontalSpeed = 1400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0"))
	float ZBoost = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0"))
	float Duration = 0.22f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", ClampMax="1"))
	float BrakingFrictionFactor = 0.30f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUse2D = true;
};