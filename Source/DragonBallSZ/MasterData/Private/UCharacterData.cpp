// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UCharacterData.h"
#include "DragonBallSZ.h"
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