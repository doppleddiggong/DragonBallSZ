// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UDBSZEventManager.h"

void UDBSZEventManager::SendMessage(const FString& InMsg)
{
	OnMessage.Broadcast(InMsg);
}

void UDBSZEventManager::SendCamera(const int& Group, const int& Index)
{
	OnCamera.Broadcast(Group, Index);
}

void UDBSZEventManager::SendUpdateHealth(const bool IsPlayer, const float CurHP, const float MaxHp)
{
	OnUpdateHealth.Broadcast(IsPlayer, CurHP, MaxHp);
}

void UDBSZEventManager::SendHitStop(AActor* Target, const EAttackPowerType Type)
{
	OnHitStop.Broadcast(Target, Type);
}

void UDBSZEventManager::SendHitStopPair(
	AActor* Attacker, const EAttackPowerType AttackerType,
	AActor* Target,   const EAttackPowerType TargetType)
{
	OnHitStop.Broadcast(Attacker, AttackerType);
	OnHitStop.Broadcast(Target,   TargetType);
}

void UDBSZEventManager::SendKnockback(AActor* Target, AActor* Instigator, EAttackPowerType Type, float Resistance)
{
	OnKnockback.Broadcast(Target, Instigator, Type, Resistance);
}

void UDBSZEventManager::SendDash(AActor* Target, bool bIsDashing)
{
	OnDash.Broadcast(Target, bIsDashing);
}

void UDBSZEventManager::SendTeleport(AActor* Target)
{
	OnTeleport.Broadcast(Target);
}

void UDBSZEventManager::SendAttack(AActor* Target, int ComboCount)
{
	OnAttack.Broadcast(Target, ComboCount);
}