// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UDashSystem.h"

#include "ACombatCharacter.h"
#include "UDBSZEventManager.h"

#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

UDashSystem::UDashSystem()
{
	PrimaryComponentTick.bCanEverTick = true;
	
}

void UDashSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FVector Vel = GetOwner()->GetVelocity();
	if (!Vel.IsNearlyZero())
	{
		FRotator BaseRot = Vel.Rotation();
		FRotator OffsetRot(90.f, 0.f, 0.f);
		if (DashWindComp) DashWindComp->SetWorldRotation(BaseRot + OffsetRot);
	}
}

void UDashSystem::InitSystem(ACombatCharacter* InOwner, UNiagaraSystem* InDashNiagaraSystem)
{
	this->Owner = InOwner;

	NiagaraComp = NewObject<UNiagaraComponent>(Owner, TEXT("DashNiagaraComp"));
	DashWindComp = NewObject<UNiagaraComponent>(Owner, TEXT("DashWindComp"));
	if (!NiagaraComp)
		return;

	NiagaraComp->SetupAttachment(Owner->GetRootComponent());
	NiagaraComp->SetAutoActivate(false);
	if (InDashNiagaraSystem)
		NiagaraComp->SetAsset(InDashNiagaraSystem);
	NiagaraComp->RegisterComponent();

	DashWindComp->SetupAttachment(Owner->GetRootComponent());
	DashWindComp->SetAutoActivate(false);
	if (Owner->DashWindVFX)
		DashWindComp->SetAsset(Owner->DashWindVFX);
	DashWindComp->RegisterComponent();
	
	auto EventManager = UDBSZEventManager::Get(GetWorld());
	EventManager->OnUpstream.AddDynamic(this, &UDashSystem::OnUpstream);
	EventManager->OnDownstream.AddDynamic(this, &UDashSystem::OnDownstream);
	EventManager->OnDash.AddDynamic(this, &UDashSystem::OnDash);
}

void UDashSystem::ActivateEffect(const bool bState)
{
	if (ActivateState == bState)
		return;

	ActivateState = bState;

	if (bState)
	{
		NiagaraComp->Activate(true);
		DashWindComp->Activate();
	}
	else
	{
		NiagaraComp->Deactivate();
		DashWindComp->DeactivateImmediate();
	}
};

void UDashSystem::OnUpstream(AActor* Target, bool bState)
{
	if (Target != Owner)
		return;

	if (bState)
		NiagaraComp->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));

	ActivateEffect(bState);
}

void UDashSystem::OnDownstream(AActor* Target, bool bState)
{
	if (Target != Owner)
		return;

	if (bState)
		NiagaraComp->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));

	ActivateEffect(bState);
}

void UDashSystem::OnDash(AActor* Target, bool bState, FVector Direction)
{
	if (Target != Owner)
		return;

	NiagaraComp->SetRelativeRotation(Direction.Rotation());
	ActivateEffect(bState);
}
