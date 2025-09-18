// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "EVFXType.h"
#include "Engine/DataAsset.h"
#include "UVFXDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FVFXData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<class UNiagaraSystem> VFXAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector LocationOffset = FVector::Zero();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRotator RotatorOffset = FRotator(0, 0, 0 );
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Scale = 1.f;
};

UCLASS(BlueprintType)
class DRAGONBALLSZ_API UVFXDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "VFX")
	TMap<EVFXType, FVFXData> VFXData;
};