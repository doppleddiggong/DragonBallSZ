// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "Components/ActorComponent.h"
#include "UDashSystem.generated.h"

UCLASS( Blueprintable, ClassGroup=(DBSZ), meta=(BlueprintSpawnableComponent) )
class DRAGONBALLSZ_API UDashSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UDashSystem();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
						   FActorComponentTickFunction* ThisTickFunction) override;

public:
	UFUNCTION(BlueprintCallable, Category="Dash")
	void InitSystem(ACharacter* InOwner, UNiagaraSystem* InDashNiagaraSystem);

	UFUNCTION(BlueprintCallable, Category="Dash")
	FORCEINLINE void ActivateEffect(bool bState)
	{
		if ( ActivateState == bState )
			return;

		ActivateState = bState;

		if (bState)
			NiagaraComp->Activate(true);
		else
			NiagaraComp->Deactivate();
	}

public:
	UPROPERTY(Transient, BlueprintReadOnly, Category="Dash")
	class ACharacter* Owner;
	
	UPROPERTY(Transient, BlueprintReadOnly, Category="Dash")
	class UNiagaraComponent* NiagaraComp = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash")
	float DashActivateScale = 1.25f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Dash")
	float DashActivateValue = 1000.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Dash")
	bool ActivateState = false;
};
