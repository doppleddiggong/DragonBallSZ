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

private:
	UFUNCTION()
	void ChargeKiTick() const;
	
public:
	UPROPERTY(Transient, BlueprintReadOnly, Category="ChargeKi")
	class ACombatCharacter* Owner;
	
	UPROPERTY(Transient, BlueprintReadOnly, Category="ChargeKi")
	class UNiagaraComponent* NiagaraComp = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="ChargeKi")
	bool ActivateState = false;

	UPROPERTY()
	class UDBSZEventManager* EventManager;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="ChargeKi")
	float ChargeDelay = 0.1f;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="ChargeKi")
	float ChargeKiAmount = 1.0f;

private:
	FTimerHandle KiChargeTimerHandle;
};
