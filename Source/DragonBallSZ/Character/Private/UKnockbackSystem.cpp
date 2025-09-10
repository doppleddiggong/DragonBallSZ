// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "UKnockbackSystem.h"
#include "UDBSZDataManager.h"
#include "FKnockbackData.h"
#include "TimerManager.h"
#include "UDBSZEventManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

static FORCEINLINE float Clamp01(float X){ return FMath::Clamp(X, 0.f, 1.f); }

UKnockbackSystem::UKnockbackSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UKnockbackSystem::BeginPlay()
{
    Super::BeginPlay();
}

void UKnockbackSystem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    RestoreMovement();
    Super::EndPlay(EndPlayReason);
}

void UKnockbackSystem::InitSystem(ACharacter* InOwner)
{
    this->Owner = InOwner;
	
    MeshComp = Owner->GetMesh();
    MoveComp = Owner->GetCharacterMovement();

	if (auto EventManager = UDBSZEventManager::Get(this))
	{
		EventManager->OnKnockback.AddDynamic(this, &UKnockbackSystem::OnKnockback);
	}
}

void UKnockbackSystem::OnKnockback(
	AActor* InOwner,
	const FHitResult& Hit, AActor* InstigatorActor,
	EAttackPowerType Type, float Resistance)
{
	if ( Owner != InOwner )
		return;

	FKnockbackData Params;
	if ( UDBSZDataManager::Get(GetWorld())->GetKnockbackData(Type, Params) == false )
		return;

	const FVector Dir = ComputeKnockDir(Hit, GetOwner(), InstigatorActor, Params.bUse2D);
	Knockback(Dir, Params, Resistance);
}

FVector UKnockbackSystem::ComputeKnockDir(const FHitResult& Hit, const AActor* Victim,
										  const AActor* InstigatorActor, bool bUse2D)
{
	// 공격 들어온 반대방향
	FVector Dir = (-Hit.ImpactNormal).GetSafeNormal();

	if (Dir.IsNearlyZero() && InstigatorActor && Victim)
		Dir = (Victim->GetActorLocation() - InstigatorActor->GetActorLocation()).GetSafeNormal();

	if (Dir.IsNearlyZero() && Hit.TraceStart != Hit.TraceEnd)
		Dir = (Hit.TraceEnd - Hit.TraceStart).GetSafeNormal();

	if (bUse2D)
	{
		Dir.Z = 0.f;
		Dir.Normalize();
	}
    
	return Dir.IsNearlyZero() ? FVector::ForwardVector : Dir;
}


void UKnockbackSystem::Knockback(const FVector& Dir, const FKnockbackData& Params, float Resistance)
{
	if (MoveComp->MovementMode != MOVE_Falling)
		MoveComp->SetMovementMode(MOVE_Falling);

	if (!bFriction)
	{
		PrevBrakingFriction = MoveComp->BrakingFrictionFactor;
		bFriction = true;
	}
	MoveComp->BrakingFrictionFactor = FMath::Clamp(Params.BrakingFrictionFactor, 0.f, 1.f);

	const float H = FMath::Max(0.f, Params.HorizontalSpeed) * (1.f - Clamp01(Resistance));
	const FVector Launch = Dir * H + FVector(0,0, FMath::Max(0.f, Params.ZBoost));

	Owner->LaunchCharacter(Launch, true, true);

	if (UWorld* W = GetWorld())
	{
		W->GetTimerManager().ClearTimer(RestoreTimer);
		W->GetTimerManager().SetTimer(
			RestoreTimer, this, &UKnockbackSystem::RestoreMovement,
			FMath::Max(0.f, Params.Duration), false
		);
	}
}

void UKnockbackSystem::RestoreMovement()
{
	if (MoveComp && bFriction)
	{
		MoveComp->BrakingFrictionFactor = PrevBrakingFriction;
		bFriction = false;
	}
}