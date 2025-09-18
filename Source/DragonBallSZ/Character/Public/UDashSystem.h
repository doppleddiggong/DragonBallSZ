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

public:
	UFUNCTION(BlueprintCallable, Category="Dash")
	void InitSystem(class ACombatCharacter* InOwner, UNiagaraSystem* InDashNiagaraSystem);

	UFUNCTION(BlueprintCallable, Category="Dash")
	void ActivateEffect(const bool bState);

	UFUNCTION(BlueprintCallable, Category="Event")
	void OnUpstream(AActor* Target, bool bState);

	UFUNCTION(BlueprintCallable, Category="Event")
	void OnDownstream(AActor* Target, bool bState);

	UFUNCTION(BlueprintCallable, Category="Event")
	void OnDash(AActor* Target, bool bState, FVector Direction);

public:
	UPROPERTY(Transient, BlueprintReadOnly, Category="Dash")
	class ACombatCharacter* Owner;
	
	UPROPERTY(Transient, BlueprintReadOnly, Category="Dash")
	class UNiagaraComponent* NiagaraComp = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash")
	float DashActivateValue = 1000.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Dash")
	bool ActivateState = false;
};
