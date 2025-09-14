// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UDBSZEventManager.h"
#include "DragonBallSZ.h"

void UDBSZEventManager::SendMessage(const FString& InMsg)
{
	PRINTLOG(TEXT("SendMessage(%s)"), *InMsg );
	
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

void UDBSZEventManager::SendCameraShake(AActor* Target, const EAttackPowerType Type)
{
	OnCameraShake.Broadcast(Target, Type);
}

void UDBSZEventManager::SendKnockback(AActor* Target, AActor* Instigator, EAttackPowerType Type, float Resistance)
{
	OnKnockback.Broadcast(Target, Instigator, Type, Resistance);
}

void UDBSZEventManager::SendDash(AActor* Target, bool bIsDashing, FVector Direction)
{
	OnDash.Broadcast(Target, bIsDashing, Direction );
}

void UDBSZEventManager::SendTeleport(AActor* Target)
{
	OnTeleport.Broadcast(Target);
}

void UDBSZEventManager::SendAttack(AActor* Target, int ComboCount)
{
	OnAttack.Broadcast(Target, ComboCount);
}

void UDBSZEventManager::SendSpecialAttack(AActor* Target, int32 SpecialIndex)
{
	OnSpecialAttack.Broadcast(Target, SpecialIndex);
}

void UDBSZEventManager::SendGuard(AActor* Target, bool bState )
{
	OnGuard.Broadcast(Target, bState);
}

void UDBSZEventManager::SendAvoid(AActor* Target, bool bState)
{
	OnAvoid.Broadcast(Target, bState);
}

void UDBSZEventManager::SendPowerCharge(AActor* Target, bool bState)
{
	OnPowerCharge.Broadcast(Target, bState);
}

void UDBSZEventManager::SendUpstream(AActor* Target, bool bStart)
{
	OnUpstream.Broadcast(Target, bStart);
}

void UDBSZEventManager::SendDownstream(AActor* Target, bool bStart)
{
	OnDownstream.Broadcast(Target, bStart);
}
