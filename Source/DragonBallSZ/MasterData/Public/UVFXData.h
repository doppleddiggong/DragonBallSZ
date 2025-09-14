// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "EVFXType.h"
#include "Engine/DataAsset.h"
#include "UVFXData.generated.h"

UCLASS(BlueprintType)
class DRAGONBALLSZ_API UVFXData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "VFX")
	TMap<EVFXType, TObjectPtr<class UNiagaraSystem>> VFXData;
};
