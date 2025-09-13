// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "UDashSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
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

	this->ActivateEffect(MoveSpeed > DashActivateValue);
}

void UDashSystem::InitSystem(ACharacter* InOwner, UNiagaraSystem* InDashNiagaraSystem)
{
	this->Owner = InOwner;

	if ( auto Movement = Owner->GetCharacterMovement() )
		DashActivateValue  = Movement->MaxWalkSpeed * DashActivateScale;
	
	NiagaraComp = NewObject<UNiagaraComponent>(Owner, TEXT("DashNiagaraComp"));
	if (!NiagaraComp)
		return;
	
	NiagaraComp->SetupAttachment(Owner->GetRootComponent());
	NiagaraComp->SetAutoActivate(false);
	if (InDashNiagaraSystem)
		NiagaraComp->SetAsset(InDashNiagaraSystem);
	NiagaraComp->RegisterComponent();
}