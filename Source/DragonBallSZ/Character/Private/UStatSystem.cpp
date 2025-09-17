// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UStatSystem.h"

#include "GameEvent.h"
#include "UDBSZEventManager.h"

UStatSystem::UStatSystem()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UStatSystem::BeginPlay()
{
	Super::BeginPlay();
}

void UStatSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UStatSystem::InitStat_Implementation(bool IsPlayer)
{
	this->bIsPlayer = IsPlayer;
	this->IsDead = false;
}

void UStatSystem::IncreaseHealth_Implementation(float InHealPoint)
{
	this->CurHP += InHealPoint;

	if( CurHP > MaxHP )
		CurHP = MaxHP;

	if (auto EventManager = UDBSZEventManager::Get(GetWorld()) )
		EventManager->SendUpdateHealth(bIsPlayer,  CurHP, MaxHP);
}

bool UStatSystem::DecreaseHealth_Implementation(float InDamagePoint)
{
	this->CurHP -= InDamagePoint;

	if( CurHP < 0 )
	{
		CurHP = 0;
		this->IsDead = true;
	}

	if (auto EventManager = UDBSZEventManager::Get(GetWorld()) )
		EventManager->SendUpdateHealth(bIsPlayer,  CurHP, MaxHP);

	return IsDead;
}