// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Core/Macro.h"
#include "EAttackPowerType.h"
#include "EVFXType.h"
#include "UVFXDataAsset.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UDBSZVFXManager.generated.h"

UCLASS()
class DRAGONBALLSZ_API UDBSZVFXManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	DEFINE_SUBSYSTEM_GETTER_INLINE(UDBSZVFXManager);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category="VFX")
	EVFXType ConvertType(EAttackPowerType Type);

	UFUNCTION(BlueprintCallable, Category="VFX")
	bool GetVFXData(EVFXType Type, FVFXData& Out) const;

	UFUNCTION(BlueprintCallable, Category="VFX")
	void ShowVFXAttackType(EAttackPowerType Type, FVector Location, FRotator Rotator, FVector Scale);

	UFUNCTION(BlueprintCallable, Category="VFX")
	void ShowVFX( EVFXType Type, FVector Location, FRotator Rotator, FVector Scale);
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	TObjectPtr<class UVFXDataAsset> VFXAsset;
	
	UPROPERTY(EditAnywhere, Category = "VFX")
	TMap<EVFXType, FVFXData> VFXDataMap; 
};
