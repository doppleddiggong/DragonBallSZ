// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "APlayerControl.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "IControllable.h"


void APlayerControl::BeginPlay()
{
	Super::BeginPlay();

	if (auto LP = GetLocalPlayer())
	{
		if (auto SubSystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (IMC_Default)
			{
				SubSystem->ClearAllMappings();
				SubSystem->AddMappingContext(IMC_Default, 0);
			}
		}
	}
}

void APlayerControl::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EIC->BindAction(IA_Move, ETriggerEvent::Triggered,  this, &APlayerControl::OnMove);
		EIC->BindAction(IA_Look, ETriggerEvent::Triggered,  this, &APlayerControl::OnLook);

		EIC->BindAction(IA_Jump, ETriggerEvent::Started,    this, &APlayerControl::OnJump);
		EIC->BindAction(IA_Dash, ETriggerEvent::Started,    this, &APlayerControl::OnDash);
		EIC->BindAction(IA_LockOn, ETriggerEvent::Started,  this, &APlayerControl::OnLockOn);
		EIC->BindAction(IA_Guard, ETriggerEvent::Started,   this, &APlayerControl::OnGuardPressed);
		EIC->BindAction(IA_Guard, ETriggerEvent::Completed, this, &APlayerControl::OnGuardReleased);
		EIC->BindAction(IA_Vanish, ETriggerEvent::Started,   this, &APlayerControl::OnVanish);
		EIC->BindAction(IA_RushAttack, ETriggerEvent::Started,   this, &APlayerControl::OnRushAttack);
		EIC->BindAction(IA_EnergyBlast, ETriggerEvent::Started,   this, &APlayerControl::OnEnergyBlast);
		EIC->BindAction(IA_ChargeKi, ETriggerEvent::Started,   this, &APlayerControl::OnChargeKiPressed);
		EIC->BindAction(IA_ChargeKi, ETriggerEvent::Completed, this, &APlayerControl::OnChargeKiReleased);
		EIC->BindAction(IA_Kamehameha, ETriggerEvent::Started,   this, &APlayerControl::OnKamehameha);
	}
}

IControllable* APlayerControl::GetControllable() const
{
	APawn* P = GetPawn();
	if (!P)
		return nullptr;

	// UObject 기반 UInterface 라면 Cast 가능
	if (IControllable* C = Cast<IControllable>(P))
		return C;

	return nullptr;
}

void APlayerControl::OnMove(const FInputActionValue& Value)
{
	if (IControllable* C = GetControllable())
		C->Cmd_Move(Value.Get<FVector2D>());
}

void APlayerControl::OnLook(const FInputActionValue& Value)
{
	if (IControllable* C = GetControllable())
		C->Cmd_Look(Value.Get<FVector2D>());
}

void APlayerControl::OnJump(const FInputActionValue&)
{
	if (IControllable* C = GetControllable())
		C->Cmd_Jump();
}

void APlayerControl::OnDash(const FInputActionValue&)
{
	if (IControllable* C = GetControllable())
		C->Cmd_Dash();
}

void APlayerControl::OnLockOn(const FInputActionValue&)
{
	if (IControllable* C = GetControllable())
		C->Cmd_LockOn();
}

void APlayerControl::OnGuardPressed(const FInputActionValue&)
{
	if (IControllable* C = GetControllable())
		C->Cmd_Guard(true);
}
void APlayerControl::OnGuardReleased(const FInputActionValue&)
{
	if (IControllable* C = GetControllable())
		C->Cmd_Guard(false);
}

void APlayerControl::OnVanish(const FInputActionValue&)
{
	if (IControllable* C = GetControllable())
		C->Cmd_Vanish();
}

void APlayerControl::OnRushAttack(const FInputActionValue&)
{
	if (IControllable* C = GetControllable())
		C->Cmd_RushAttack();
}

void APlayerControl::OnEnergyBlast(const FInputActionValue&)
{
	if (IControllable* C = GetControllable())
		C->Cmd_EnergyBlast();
}

void APlayerControl::OnChargeKiPressed(const FInputActionValue&)
{
	if (IControllable* C = GetControllable())
		C->Cmd_ChargeKi(true);
}

void APlayerControl::OnChargeKiReleased(const FInputActionValue&)
{
	if (IControllable* C = GetControllable())
		C->Cmd_ChargeKi(false);
}

// -------- Kamehameha --------
void APlayerControl::OnKamehameha(const FInputActionValue&)
{
	if (IControllable* C = GetControllable())
		C->Cmd_Kamehameha();
}