// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "UDashSystem.h"

#include "ACombatCharacter.h"
#include "APlayerActor.h"
#include "UDBSZEventManager.h"
#include "DragonBallSZ.h"

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

void UDashSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	float MoveSpeed = Owner->GetVelocity().Length();

	if ( MoveSpeed > DashActivateValue )
		PRINTLOG( TEXT("Dash Activate : %f, %f"), MoveSpeed, DashActivateValue);
	
	this->ActivateEffect(MoveSpeed > DashActivateValue);
}

void UDashSystem::InitSystem(ACombatCharacter* InOwner, UNiagaraSystem* InDashNiagaraSystem)
{
	this->Owner = InOwner;

	NiagaraComp = NewObject<UNiagaraComponent>(Owner, TEXT("DashNiagaraComp"));
	if (!NiagaraComp)
		return;
	
	NiagaraComp->SetupAttachment(Owner->GetRootComponent());
	NiagaraComp->SetAutoActivate(false);
	if (InDashNiagaraSystem)
		NiagaraComp->SetAsset(InDashNiagaraSystem);
	NiagaraComp->RegisterComponent();

	auto EventManager = UDBSZEventManager::Get(GetWorld());
	EventManager->OnUpstream.AddDynamic(this, &UDashSystem::OnUpstream);
	EventManager->OnDownstream.AddDynamic(this, &UDashSystem::OnDownstream);
	EventManager->OnDash.AddDynamic(this, &UDashSystem::OnDash);
}

void UDashSystem::ActivateEffect(const bool bState)
{
	if ( ActivateState == bState )
		return;

	ActivateState = bState;

	if (bState)
	{
		NiagaraComp->Activate(true);
	}
	else
	{
		NiagaraComp->Deactivate();
	}

	if ( auto Player = Cast<APlayerActor>(Owner))
	{
		const TCHAR* PrintMsg = bState ? TEXT("Dash Activate Start") : TEXT("Dash Deactivate End");
		PRINT_STRING(TEXT("%s"), PrintMsg);
	}
};

void UDashSystem::OnUpstream(AActor* Target, bool bState)
{
	if( Target != Owner )
		return;
	
	ActivateEffect(bState);
}

void UDashSystem::OnDownstream(AActor* Target, bool bState)
{
	if( Target != Owner )
		return;

	ActivateEffect(bState);
}

void UDashSystem::OnDash(AActor* Target, bool bState, FVector Direction)
{
	if( Target != Owner )
		return;

	ActivateEffect(bState);
}