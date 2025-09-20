// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UChargeKiSystem.generated.h"

UCLASS( Blueprintable, ClassGroup=(DBSZ), meta=(BlueprintSpawnableComponent) )
class DRAGONBALLSZ_API UChargeKiSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UChargeKiSystem();

protected:
	virtual void BeginPlay() override;


public:
	UFUNCTION(BlueprintCallable, Category="ChargeKi")
	void InitSystem(class ACombatCharacter* InOwner, UNiagaraSystem* InNiagaraSystem);

	UFUNCTION(BlueprintCallable, Category="ChargeKi")
	void ActivateEffect(const bool bState);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="ChargeKi")
	FORCEINLINE bool IsActivateState()
	{
		return ActivateState;
	}
private:
	UFUNCTION()
	void ChargeKiTick();
	
public:
	UPROPERTY(Transient, BlueprintReadOnly, Category="ChargeKi")
	TObjectPtr<class ACombatCharacter> Owner;
	
	UPROPERTY(Transient, BlueprintReadOnly, Category="ChargeKi")
	TObjectPtr<class UNiagaraComponent> NiagaraComp = nullptr;

	UPROPERTY()
	TObjectPtr<class UDBSZEventManager> EventManager;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="ChargeKi")
	float ChargeDelay = 0.1f;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="ChargeKi")
	float ChargeKiAmount = 1.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="ChargeKi")
	float AccelerationFactor = 0.1f;
	
private:
	FTimerHandle KiChargeTimerHandle;
	float CurrentChargeTime = 0.0f;

	bool ActivateState = false;
};
