// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "UDashSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

UDashSystem::UDashSystem()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UDashSystem::BeginPlay()
{
	Super::BeginPlay();
}

void UDashSystem::InitDash( AActor* Owner, UNiagaraSystem* DashNiagaraSystem )
{
	NiagaraComp = NewObject<UNiagaraComponent>(Owner, TEXT("DashNiagaraComp"));
	if (!NiagaraComp)
		return;
	
	NiagaraComp->SetupAttachment(Owner->GetRootComponent());
	NiagaraComp->SetAutoActivate(false);
	if (DashNiagaraSystem)
		NiagaraComp->SetAsset(DashNiagaraSystem);
	NiagaraComp->RegisterComponent();
}