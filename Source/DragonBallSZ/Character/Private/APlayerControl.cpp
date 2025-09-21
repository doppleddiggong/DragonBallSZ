// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "APlayerControl.h"
#include "IControllable.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

#include "InputMappingContext.h"
#include "InputAction.h"

#include "Shared/FComponentHelper.h"


#define IMC_DEFAULT_PATH			TEXT("/Game/CustomContents/Input/IMC_DBSZ_Player.IMC_DBSZ_Player")
#define IA_MOVE_PATH				TEXT("/Game/CustomContents/Input/IA_DBSZ_Movement.IA_DBSZ_Movement")
#define IA_LOOK_PATH				TEXT("/Game/CustomContents/Input/IA_DBSZ_LookAround.IA_DBSZ_LookAround")
#define IA_ALTITUDE_UP_PATH			TEXT("/Game/CustomContents/Input/IA_DBSZ_AltitudeUp.IA_DBSZ_AltitudeUp")
#define IA_ALTITUDE_DOWN_PATH		TEXT("/Game/CustomContents/Input/IA_DBSZ_AltitudeDown.IA_DBSZ_AltitudeDown")
#define IA_JUMP_PATH				TEXT("/Game/CustomContents/Input/IA_DBSZ_Jump.IA_DBSZ_Jump")
#define IA_DASH_PATH				TEXT("/Game/CustomContents/Input/IA_DBSZ_Dash.IA_DBSZ_Dash")
#define IA_LANDING_PATH				TEXT("/Game/CustomContents/Input/IA_DBSZ_Landing.IA_DBSZ_Landing")
#define IA_GUARD_PATH				TEXT("/Game/CustomContents/Input/IA_DBSZ_Guard.IA_DBSZ_Guard")
#define IA_VANISH_PATH				TEXT("/Game/CustomContents/Input/IA_DBSZ_Vanish.IA_DBSZ_Vanish")
#define IA_RUSH_ATTACK_PATH			TEXT("/Game/CustomContents/Input/IA_DBSZ_RushAttack.IA_DBSZ_RushAttack")
#define IA_ENERGY_BLAST_PATH		TEXT("/Game/CustomContents/Input/IA_DBSZ_EnergyBlast.IA_DBSZ_EnergyBlast")
#define IA_CHARGE_KI_PATH			TEXT("/Game/CustomContents/Input/IA_DBSZ_CharageKi.IA_DBSZ_CharageKi")
#define IA_KAMEHAMEHA_PATH			TEXT("/Game/CustomContents/Input/IA_DBSZ_Kamehameha.IA_DBSZ_Kamehameha")

APlayerControl::APlayerControl()
{
	IMC_Default = FComponentHelper::LoadAsset<UInputMappingContext>(IMC_DEFAULT_PATH);

	IA_Move = FComponentHelper::LoadAsset<UInputAction>(IA_MOVE_PATH);
	IA_Look = FComponentHelper::LoadAsset<UInputAction>(IA_LOOK_PATH);
	IA_AltitudeUp = FComponentHelper::LoadAsset<UInputAction>(IA_ALTITUDE_UP_PATH);
	IA_AltitudeDown = FComponentHelper::LoadAsset<UInputAction>(IA_ALTITUDE_DOWN_PATH);
	IA_Jump = FComponentHelper::LoadAsset<UInputAction>(IA_JUMP_PATH);
	IA_Dash = FComponentHelper::LoadAsset<UInputAction>(IA_DASH_PATH);
	IA_Landing = FComponentHelper::LoadAsset<UInputAction>(IA_LANDING_PATH);
	IA_Guard = FComponentHelper::LoadAsset<UInputAction>(IA_GUARD_PATH);
	IA_Vanish = FComponentHelper::LoadAsset<UInputAction>(IA_VANISH_PATH);
	IA_RushAttack = FComponentHelper::LoadAsset<UInputAction>(IA_RUSH_ATTACK_PATH);
	IA_EnergyBlast = FComponentHelper::LoadAsset<UInputAction>(IA_ENERGY_BLAST_PATH);
	IA_ChargeKi = FComponentHelper::LoadAsset<UInputAction>(IA_CHARGE_KI_PATH);
	IA_Kamehameha = FComponentHelper::LoadAsset<UInputAction>(IA_KAMEHAMEHA_PATH);
}

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

		EIC->BindAction(IA_AltitudeUp, ETriggerEvent::Started,  this, &APlayerControl::OnAltitudeUp);
		EIC->BindAction(IA_AltitudeUp, ETriggerEvent::Completed, this, &APlayerControl::OnAltitudeReleased);
		EIC->BindAction(IA_AltitudeUp, ETriggerEvent::Canceled, this, &APlayerControl::OnAltitudeReleased);
		
		EIC->BindAction(IA_AltitudeDown, ETriggerEvent::Started,  this, &APlayerControl::OnAltitudeDown);
		EIC->BindAction(IA_AltitudeDown, ETriggerEvent::Completed, this, &APlayerControl::OnAltitudeReleased);
		EIC->BindAction(IA_AltitudeDown, ETriggerEvent::Canceled, this, &APlayerControl::OnAltitudeReleased);

		
		EIC->BindAction(IA_Jump, ETriggerEvent::Started,    this, &APlayerControl::OnJump);
		EIC->BindAction(IA_Dash, ETriggerEvent::Started,    this, &APlayerControl::OnDash);
		EIC->BindAction(IA_Landing, ETriggerEvent::Started,  this, &APlayerControl::OnLanding);
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

void APlayerControl::OnAltitudeUp(const FInputActionValue& Value)
{
	if (IControllable* C = GetControllable())
		C->Cmd_AltitudeUp();
}

void APlayerControl::OnAltitudeDown(const FInputActionValue& Value)
{
	if (IControllable* C = GetControllable())
		C->Cmd_AltitudeDown();
}

void APlayerControl::OnAltitudeReleased(const FInputActionValue& Value)
{
	if (IControllable* C = GetControllable())
		C->Cmd_AltitudeReleased();
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

void APlayerControl::OnLanding(const FInputActionValue&)
{
	if (IControllable* C = GetControllable())
		C->Cmd_Landing();
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