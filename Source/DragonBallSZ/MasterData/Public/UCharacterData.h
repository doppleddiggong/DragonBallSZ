// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "EAttackPowerType.h"
#include "Engine/DataAsset.h"
#include "UCharacterData.generated.h"

USTRUCT(BlueprintType)
struct FRushData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EAttackPowerType PowerType = EAttackPowerType::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UAnimMontage> AttackAsset;
};

UCLASS()
class DRAGONBALLSZ_API UCharacterData : public UDataAsset
{
	GENERATED_BODY()

public:
	bool LoadRushAttackMontage(TArray<TObjectPtr<UAnimMontage>>& OutMontage, TArray<EAttackPowerType>& OutPowerType ) const;
	bool LoadDashMontage( TObjectPtr<UAnimMontage>& OutDashMontage) const;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FRushData> RushData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UAnimMontage> DashAsset;

	/*
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UAnimMontage> EnergyBlastAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TSoftObjectPtr<UAnimMontage>> HitAsset;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UAnimMontage> DeathAsset;
	*/
};