// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UStatSystem.h"

#include "UDBSZDataManager.h"
#include "UDBSZEventManager.h"

#define MIN_DMG_MUL 0.85f
#define MAX_DMG_MUL 1.15f

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

void UStatSystem::InitStat_Implementation(const bool InIsPlayer, const ECharacterType InCharacterType)
{
	this->bIsPlayer = InIsPlayer;
	this->CharacterType = InCharacterType;
	this->bIsDead = false;

	FCharacterInfoData Params;
	if ( UDBSZDataManager::Get(GetWorld())->GetCharacterInfoData(CharacterType, Params) )
	{
		this->MaxHP = Params.MaxHP;
		this->CurHP = this->MaxHP;

		this->AttackDamage = Params.AttackDamage;
		this->AttackChargeKi = Params.AttackChargeKi;
		
		this->MaxKi = Params.MaxKi;
		this->CurKi = Params.StartKi;

		this->BlastNeedKi = Params.BlastNeedKi;
		this->BlastDamage = Params.BlastDamage;
		this->BlastShotDelay = Params.BlastShotDelay;
		
		this->KamehameNeedKi = Params.KamehameNeedKi;
		this->KamehameDamage = Params.KamehameDamage;
	
		this->SightLength= Params.SightLength;
		this->SightAngle = Params.SightAngle;
	}
}

float UStatSystem::GetRandDmg(float Damage)
{
	return FMath::RandRange( Damage * MIN_DMG_MUL, Damage * MAX_DMG_MUL);
}

float UStatSystem::GetAttackDamage(int ComboCount)
{
	if ( AttackDamage.IsValidIndex(ComboCount) )
		return GetRandDmg( AttackDamage[ComboCount]);

	return 0;
}	

float UStatSystem::GetBlastDamage()
{
	return GetRandDmg(BlastDamage);
}

float UStatSystem::GetKamehameDamage()
{
	return GetRandDmg(KamehameDamage);
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
		this->bIsDead = true;
	}

	if (auto EventManager = UDBSZEventManager::Get(GetWorld()) )
		EventManager->SendUpdateHealth(bIsPlayer,  CurHP, MaxHP);

	return bIsDead;
}

void UStatSystem::IncreaseKi_Implementation(float InKi)
{
	this->CurKi += InKi;

	if( CurKi > MaxKi )
		CurKi = MaxKi;

	if (auto EventManager = UDBSZEventManager::Get(GetWorld()) )
		EventManager->SendUpdateKi(bIsPlayer, CurKi, MaxKi);
}

void UStatSystem::DecreaseKi_Implementation(float InKi)
{
	this->CurKi -= InKi;

	if( CurKi < 0 )
		CurKi = 0;

	if (auto EventManager = UDBSZEventManager::Get(GetWorld()) )
		EventManager->SendUpdateKi(bIsPlayer, CurKi, MaxKi);
}