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

void UDBSZEventManager::SendHitStop(AActor* Target, const FHitStopParams& Params)
{
	OnHitStop.Broadcast(Target, Params);
}

void UDBSZEventManager::SendHitStopPair(
	AActor* Attacker, const FHitStopParams& ForAtk,
	AActor* Target,   const FHitStopParams& ForTarget)
{
	OnHitStop.Broadcast(Attacker, ForAtk);
	OnHitStop.Broadcast(Target,   ForTarget);
}