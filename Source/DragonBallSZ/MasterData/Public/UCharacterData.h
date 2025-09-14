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
	bool LoadHitMontage( TArray<TObjectPtr<UAnimMontage>>& OutHitMontage) const;
	bool LoadDeathMontage( TObjectPtr<UAnimMontage>& OutDeathMontage) const;

	bool LoadDashVFX( TObjectPtr<class UNiagaraSystem>& OutDashVFX) const;
	bool LoadEnergyBlast(TSubclassOf<class AEnergyBlastActor>& OutEnergyBlast);

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Montage)
	TArray<FRushData> RushData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Montage)
	TSoftObjectPtr<UAnimMontage> DashAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Montage)
	TArray<TSoftObjectPtr<UAnimMontage>> HitAsset;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Montage)
	TSoftObjectPtr<UAnimMontage> DeathAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=VFX)
	TSoftObjectPtr<class UNiagaraSystem> DashVFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Actor)
	TSubclassOf<class AEnergyBlastActor> EnergyBlast;
};