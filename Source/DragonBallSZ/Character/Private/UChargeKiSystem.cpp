// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UChargeKiSystem.h"

#include "ACombatCharacter.h"
#include "EAnimMontageType.h"
#include "UDBSZEventManager.h"
#include "UStatSystem.h" // Added for UStatSystem access

#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Kismet/GameplayStatics.h" // For GetWorld() and TimerManager

UChargeKiSystem::UChargeKiSystem()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UChargeKiSystem::BeginPlay()
{
	Super::BeginPlay();
}

void UChargeKiSystem::InitSystem(ACombatCharacter* InOwner, UNiagaraSystem* InNiagaraSystem)
{
	this->Owner = InOwner;

	NiagaraComp = NewObject<UNiagaraComponent>(Owner, TEXT("ChargeKiNiagaraComp"));
	if (!NiagaraComp)
		return;
	
	NiagaraComp->SetupAttachment(Owner->GetRootComponent());
	NiagaraComp->SetAutoActivate(false);
	if (InNiagaraSystem)
		NiagaraComp->SetAsset(InNiagaraSystem);
	NiagaraComp->RegisterComponent();

	EventManager = UDBSZEventManager::Get(GetWorld());
}

void UChargeKiSystem::ActivateEffect(const bool bState)
{
	if ( ActivateState == bState )
		return;

	ActivateState = bState;

	if (bState)
	{
		EventManager->SendPowerCharge(Owner, true);

		NiagaraComp->Activate(true);
		Owner->SetChargeKi(true);
		Owner->PlayTypeMontage(EAnimMontageType::ChargeKi);

		GetWorld()->GetTimerManager().SetTimer(
			KiChargeTimerHandle,
			this,
			&UChargeKiSystem::ChargeKiTick,
			ChargeDelay,
			true
		);
	}
	else
	{
		EventManager->SendPowerCharge(Owner, false);

		NiagaraComp->DeactivateImmediate();
		Owner->SetChargeKi(false);
		Owner->StopTargetMontage(EAnimMontageType::ChargeKi, 0.15f);

		if (GetWorld())
			GetWorld()->GetTimerManager().ClearTimer(KiChargeTimerHandle);
	}
	
	EventManager->SendPowerCharge(Owner, bState);
}

void UChargeKiSystem::ChargeKiTick() const
{
	Owner->StatSystem->IncreaseKi(ChargeKiAmount);
}