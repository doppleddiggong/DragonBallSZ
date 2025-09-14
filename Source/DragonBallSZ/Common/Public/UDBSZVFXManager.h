// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Core/Macro.h"
#include "EVFXType.h"
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
	void ShowVFX( EVFXType Type, FVector Location, FRotator Rotator, FVector Scale);
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	TObjectPtr<class UVFXData> VFXAsset;
	
	UPROPERTY(EditAnywhere, Category = "VFX")
	TMap<EVFXType, TObjectPtr<class UNiagaraSystem>> VFXData; 
};
