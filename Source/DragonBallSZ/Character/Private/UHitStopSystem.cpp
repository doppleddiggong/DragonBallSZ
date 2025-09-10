﻿// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "UHitStopSystem.h"

#include "UDBSZEventManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UHitStopSystem::UHitStopSystem()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bTickEvenWhenPaused = true;
}

void UHitStopSystem::BeginPlay()
{
	Super::BeginPlay();
}

void UHitStopSystem::InitSystem(ACharacter* InOwner )
{
	this->Owner = InOwner;
	
	MeshComp = Owner->GetMesh();
	MoveComp = Owner->GetCharacterMovement();

	// 구독
	if (auto EM = UDBSZEventManager::Get(this))
	{
		EM->OnHitStop.AddDynamic(this, &UHitStopSystem::OnHitStopIssued);
	}
}

void UHitStopSystem::TickComponent(float DeltaTime, ELevelTick TickType,
									  FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if ( !bActive )
		return;

	const double Now = GetWorld()->GetRealTimeSeconds();
	if (Now >= EndRealTimeSeconds)
		EndFreeze();
}

void UHitStopSystem::OnHitStopIssued(AActor* Target, const EAttackPowerType Type)
{
	if (Target != Owner)
		return;

	ApplyHitStop(Type);
}

void UHitStopSystem::ApplyHitStop(const EAttackPowerType Type)
{
	const double Now = GetWorld()->GetRealTimeSeconds();

	auto Params = FHitStopParams::GetParamsFromType(Type);
	
	if (!bActive)
	{
		BeginFreeze(Params);
		EndRealTimeSeconds = Now + Params.Duration;
		LastParams = Params;
		return;
	}

	const bool Stronger = Params.TimeDilation < LastParams.TimeDilation;
	const bool Longer   = (Now + Params.Duration) > EndRealTimeSeconds;

	if (Params.bRefreshIfStronger ? (Stronger || Longer) : Longer)
	{
		if (Stronger)
			Owner->CustomTimeDilation = FMath::Clamp(Params.TimeDilation, 0.001f, 1.0f);

		EndRealTimeSeconds = Now + Params.Duration;
		LastParams = Params;
	}
}

void UHitStopSystem::BeginFreeze(const FHitStopParams& Params)
{
	bActive = true;

	SavedCustomTimeDilation = Owner->CustomTimeDilation;

	Owner->CustomTimeDilation = FMath::Clamp(Params.TimeDilation, 0.001f, 1.0f);
	MoveComp->StopMovementImmediately();
}

void UHitStopSystem::EndFreeze()
{
	Owner->CustomTimeDilation = SavedCustomTimeDilation;
	bActive = false;
}