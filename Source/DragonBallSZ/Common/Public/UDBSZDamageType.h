// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DamageType.h"
#include "EAttackPowerType.h"
#include "UDBSZDamageType.generated.h"

UCLASS()
class DRAGONBALLSZ_API UDBSZDamageType : public UDamageType
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Damage")
	EAttackPowerType AttackPowerType;
};
