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

public:
	UFUNCTION(BlueprintCallable, Category="Dash")
	void InitDash(AActor* Owner, UNiagaraSystem* DashNiagaraSystem);

	UFUNCTION(BlueprintCallable, Category="Dash")
	FORCEINLINE void ActivateEffect(bool State)
	{
		NiagaraComp->Activate(State);
	}

public:
	UPROPERTY(Transient, BlueprintReadOnly, Category="Dash")
	class UNiagaraComponent* NiagaraComp = nullptr;
};
