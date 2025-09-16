// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UCharacterData.h"
#include "DragonBallSZ.h"
#include "AEnergyBlastActor.h"

#include "NiagaraSystem.h"
#include "Animation/AnimMontage.h"

bool UCharacterData::LoadRushAttackMontage(
	TArray<TObjectPtr<UAnimMontage>>& OutMontage,
	TArray<EAttackPowerType>& OutPowerType) const
{
	OutMontage.Empty();
	OutPowerType.Empty();

	for (const FRushData& Data : RushData)
	{
		UAnimMontage* LoadedMontage = Data.AttackAsset.LoadSynchronous();
		if (LoadedMontage)
		{
			OutMontage.Add(LoadedMontage);
			OutPowerType.Add(Data.PowerType);
		}
		else
		{
			PRINTLOG( TEXT("Failed to LoadRushAttackMontage"));
			return false;
		}
	}

	return true;
}

bool UCharacterData::LoadDashMontage( TObjectPtr<UAnimMontage>& OutDashMontage ) const
{
	OutDashMontage = DashAsset.LoadSynchronous();
	if (!OutDashMontage)
	{
		PRINTLOG(TEXT("Failed to LoadDashMontage"));
		return false;
	}

	return true;
}

bool UCharacterData::LoadHitMontage(TArray<TObjectPtr<UAnimMontage>>& OutHitMontage) const
{
	OutHitMontage.Empty();

	for (const TSoftObjectPtr<UAnimMontage>& HitAssetPtr : HitAsset)
	{
		UAnimMontage* LoadedMontage = HitAssetPtr.LoadSynchronous();
		if (LoadedMontage)
		{
			OutHitMontage.Add(LoadedMontage);
		}
		else
		{
			PRINTLOG(TEXT("Failed to LoadHitMontage"));
			return false;
		}
	}
	return true;
}

bool UCharacterData::LoadDeathMontage(TObjectPtr<UAnimMontage>& OutDeathMontage) const
{
	OutDeathMontage = DeathAsset.LoadSynchronous();
	if (!OutDeathMontage)
	{
		PRINTLOG(TEXT("Failed to LoadDeathMontage"));
		return false;
	}
	return true;
}

bool UCharacterData::LoadDashVFX(TObjectPtr<UNiagaraSystem>& OutDashVFX) const
{
	OutDashVFX = DashVFX.LoadSynchronous();
	if (!OutDashVFX)
	{
		PRINTLOG(TEXT("Failed to LoadDashVFX"));
		return false;
	}
	return true;
}

bool UCharacterData::LoadEnergyBlast(TSubclassOf<AEnergyBlastActor>& OutEnergyBlast)
{
	if ( !EnergyBlast)
	{
		PRINTLOG(TEXT("Failed to LoadEnergyBlast"));
	}

	OutEnergyBlast = this->EnergyBlast;
	return true;
}
