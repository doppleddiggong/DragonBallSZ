// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "UKnockbackSystem.h"
#include "ACombatCharacter.h"

#include "DragonBallSZ.h"
#include "UDBSZDataManager.h"
#include "FKnockbackData.h"
#include "TimerManager.h"
#include "UDBSZEventManager.h"
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
    GetWorld()->GetTimerManager().ClearTimer(FlyingTimer);
    Super::EndPlay(EndPlayReason);
}

void UKnockbackSystem::InitSystem(ACombatCharacter* InOwner)
{
    this->Owner = InOwner;
	
    MeshComp = Owner->GetMesh();
    MoveComp = Owner->GetCharacterMovement();

	if (auto EventManager = UDBSZEventManager::Get(this))
	{
		EventManager->OnKnockback.AddDynamic(this, &UKnockbackSystem::OnKnockback);
	}
}

void UKnockbackSystem::OnKnockback( AActor* Target, AActor* Instigator, EAttackPowerType Type, float Resistance)
{
	if ( Owner != Target )
		return;

	Knockback(Target, Instigator, Type, Resistance);
}

FVector UKnockbackSystem::ComputeKnockDir(const AActor* Target, const AActor* Instigator)
{
    FVector Dir = FVector::ZeroVector;

    if (Target && Instigator)
        Dir = Target->GetActorLocation() - Instigator->GetActorLocation();

    if (!Dir.Normalize())
    {
        if (Target)
        {
            FVector Fwd = Target->GetActorForwardVector();
            Dir = (-Fwd).GetSafeNormal();
        }
        else
        {
            Dir = FVector::ForwardVector;
        }
    }

    return Dir;
}

void UKnockbackSystem::Knockback(AActor* Target, AActor* Instigator, EAttackPowerType Type, float Resistance)
{
    FKnockbackData Params;
    if (auto DataManager = UDBSZDataManager::Get(GetWorld()))
    {
	    if (!DataManager->GetKnockbackData(Type, Params))
	    	return;
    }

    const FVector Dir = ComputeKnockDir(Target, Instigator);

    if (MoveComp->MovementMode != MOVE_Falling)
        MoveComp->SetMovementMode(MOVE_Falling);
	
	if (!bFriction)
	{
		PrevBrakingFriction = MoveComp->BrakingFrictionFactor;
		bFriction = true;
	}
	MoveComp->BrakingFrictionFactor = FMath::Clamp(Params.BrakingFrictionFactor, 0.f, 1.f);

    const float Power = FMath::Max(0.f, Params.KnockbackPower) * (1.f - Clamp01(Resistance));
    
    // --- Debugging Start ---
    // PRINTLOG(TEXT("Knockback Debug: Params.UpPower = %f"), Params.UpPower);
    // PRINTLOG(TEXT("Knockback Debug: Initial Launch (Dir * Power) = %s"), *(Dir * Power).ToString());
    // PRINTLOG(TEXT("Knockback Debug: Character MovementMode before launch = %s"), *UEnum::GetValueAsString(MoveComp->MovementMode));
    // PRINTLOG(TEXT("Knockback Debug: IsFalling() = %s"), (MoveComp->IsFalling() ? TEXT("true") : TEXT("false")));
    // --- Debugging End ---

    FVector Launch = Dir * Power;
    Launch.Z += Params.UpPower; // Always apply UpPower for testing

    // --- Debugging Start ---
    // PRINTLOG(TEXT("Knockback Debug: Final Launch Vector = %s"), *Launch.ToString());
    // PRINTLOG(TEXT("Knockback Debug: Character Velocity before LaunchCharacter = %s"), *MoveComp->Velocity.ToString());
    // --- Debugging End ---

    Owner->LaunchCharacter(Launch, true, true);

    // --- Debugging Start ---
    // PRINTLOG(TEXT("Knockback Debug: Character Velocity after LaunchCharacter = %s"), *MoveComp->Velocity.ToString());
    // --- Debugging End ---    GetWorld()->GetTimerManager().ClearTimer(RestoreTimer);
    GetWorld()->GetTimerManager().SetTimer(
        RestoreTimer, this, &UKnockbackSystem::RestoreMovement,
        FMath::Max(0.f, Params.Duration), false
    );

    if (Params.bAfterFlying)
    {
        GetWorld()->GetTimerManager().ClearTimer(FlyingTimer);
        GetWorld()->GetTimerManager().SetTimer(
            FlyingTimer, this, &UKnockbackSystem::EnterFlying,
            FMath::Max(0.f, Params.FlyingDelay), false
        );
    }
}

void UKnockbackSystem::RestoreMovement()
{
	if (bFriction)
	{
		MoveComp->BrakingFrictionFactor = PrevBrakingFriction;
		bFriction = false;
	}
}

void UKnockbackSystem::EnterFlying()
{
	Owner->SetFlying();
}
