// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UCameraShakeSystem.generated.h"

UCLASS( Blueprintable, ClassGroup=(DBSZ), meta=(BlueprintSpawnableComponent) )
class DRAGONBALLSZ_API UCameraShakeSystem : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCameraShakeSystem();

public:	
	void InitSystem(class ACombatCharacter* InOwner);

	UPROPERTY(EditDefaultsOnly, Category="CameraMotion")
	TMap<EAttackPowerType, TSubclassOf<UCameraShakeBase>> CameraShakeMap;
	
private:
	UFUNCTION()
	void OnCameraShake(AActor* Target, EAttackPowerType Type);

	UPROPERTY()
	class ACombatCharacter* Owner;
};