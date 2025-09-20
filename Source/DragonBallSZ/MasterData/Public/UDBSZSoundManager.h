// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Core/Macro.h"
#include "ESoundType.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UDBSZSoundManager.generated.h"

UCLASS()
class DRAGONBALLSZ_API UDBSZSoundManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	DEFINE_SUBSYSTEM_GETTER_INLINE(UDBSZSoundManager);

	UDBSZSoundManager();
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category="Sound")
	void PlaySound(ESoundType Type, FVector Location);

	UFUNCTION(BlueprintCallable, Category="Sound")
	void PlaySound2D(ESoundType Type);

private:
	UPROPERTY(EditDefaultsOnly, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USoundData> SoundAsset;
	
	UPROPERTY(EditAnywhere, Category = "Sound")
	TMap<ESoundType, TObjectPtr<class USoundBase>> SoundData;
};
