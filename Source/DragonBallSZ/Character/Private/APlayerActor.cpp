// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "APlayerActor.h"
#include "AEnemyActor.h"

#include "Core/Macro.h"

#include "DragonBallSZ.h"
#include "UDBSZEventManager.h"

#include "UStatSystem.h"
#include "UHitStopSystem.h"
#include "URushAttackSystem.h"
#include "UDashSystem.h"
#include "UFlySystem.h"
#include "UKnockbackSystem.h"

#include "Components/ArrowComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

APlayerActor::APlayerActor()
{
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, 270.0f, 0.0f));

	StatSystem			= CreateDefaultSubobject<UStatSystem>(TEXT("StatSystem"));
	HitStopSystem		= CreateDefaultSubobject<UHitStopSystem>(TEXT("HitStopSystem"));
	KnockbackSystem		= CreateDefaultSubobject<UKnockbackSystem>(TEXT("KnockbackSystem"));
	RushAttackSystem	= CreateDefaultSubobject<URushAttackSystem>(TEXT("RushAttackSystem"));
	DashSystem			= CreateDefaultSubobject<UDashSystem>(TEXT("DashSystem"));
	FlySystem			= CreateDefaultSubobject<UFlySystem>(TEXT("FlySystem"));

	LeftHandComp = CreateDefaultSubobject<UArrowComponent>(TEXT("LeftHandComp"));
	LeftHandComp->SetupAttachment(GetMesh(), TEXT("hand_l"));

	RightHandComp = CreateDefaultSubobject<UArrowComponent>(TEXT("RightHandComp"));
	RightHandComp->SetupAttachment(GetMesh(), TEXT("hand_r"));
	RightHandComp->SetRelativeRotation(FRotator(0, -180.f, 0.f));

	LeftFootComp = CreateDefaultSubobject<UArrowComponent>(TEXT("LeftFootComp"));
	LeftFootComp->SetupAttachment(GetMesh(), TEXT("foot_l"));
	LeftFootComp->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));

	RightFootComp = CreateDefaultSubobject<UArrowComponent>(TEXT("RightFootComp"));
	RightFootComp->SetupAttachment(GetMesh(), TEXT("foot_r"));
	
	bUseControllerRotationYaw = true;
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->bOrientRotationToMovement = false;

		// 플라잉 모드 미끄러짐 방지 설정
		// 높은 값으로 설정하여 즉시 멈추도록 함
		Move->BrakingDecelerationFlying = 4096.0f; // 기본값 0.0f
    
		// 추가적으로 마찰력도 조정 가능
		Move->BrakingFriction = 4.0f; // 기본값 0.0f
	}
}

void APlayerActor::BeginPlay()
{
	Super::BeginPlay();

	if ( AActor* FoundActor = UGameplayStatics::GetActorOfClass( GetWorld(), AEnemyActor::StaticClass() ) )
		TargetActor = Cast<AEnemyActor>(FoundActor);

	StatSystem->InitStat(true);
	RushAttackSystem->InitSystem(this);
	RushAttackSystem->SetDamage( StatSystem->Damage );
	KnockbackSystem->InitSystem(this);
	
	DashSystem->InitSystem(this, DashNiagaraSystem);
	FlySystem->InitSystem(this, BIND_DYNAMIC_DELEGATE(FEndCallback, this, APlayerActor, OnFlyEnd));
	HitStopSystem->InitSystem(this);
	
	if (auto EventManager = UDBSZEventManager::Get(GetWorld()))
		EventManager->SendUpdateHealth(true, StatSystem->CurHP, StatSystem->MaxHP);
}

void APlayerActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerActor::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void APlayerActor::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (FlySystem)
		FlySystem->OnLand(Hit);
}

bool APlayerActor::IsMoveEnable_Implementation()
{
	if ( !IsControlEnable() )
		return false;

	if ( IsAttackIng() )
		return false;

	return true;
}


bool APlayerActor::IsControlEnable_Implementation()
{
	if ( IsHit )
		return false;

	if ( StatSystem->IsDead )
		return false;

	return true;
}

bool APlayerActor::IsAttackIng_Implementation()
{
	if ( RushAttackSystem->bIsAttacking || RushAttackSystem->bIsDashing )
		return true;
	return false;
}

void APlayerActor::OnFlyEnd_Implementation()
{
	DashSystem->ActivateEffect(false);
}

void APlayerActor::Cmd_Move_Implementation(const FVector2D& Axis)
{
	if ( !IsMoveEnable() )
		return;
	
	// if (Controller)
	// {
	// 	// Move By Control
	// 	// const FRotator ControlRot = Controller->GetControlRotation();
	// 	// const FRotator YawRot(0.f, ControlRot.Yaw, 0.f);
	// 	// const FVector Forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
	// 	// const FVector Right   = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);
	// }

	{
		// Move By Actor
		const FVector Forward = GetActorForwardVector();
		const FVector Right   = GetActorRightVector();

		AddMovementInput(Forward, Axis.Y);
		AddMovementInput(Right,   Axis.X);
	}
}

void APlayerActor::Cmd_Look_Implementation(const FVector2D& Axis)
{
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void APlayerActor::Cmd_Jump_Implementation()
{
	if ( !IsMoveEnable() )
		return;

	FlySystem->OnJump();
}

void APlayerActor::Cmd_Dash_Implementation()
{
	if ( !IsMoveEnable() )
		return;

	PRINTINFO();
}

void APlayerActor::Cmd_Landing_Implementation()
{
	if ( !IsControlEnable() )
		return;

	FHitResult HitResult;
	FlySystem->OnLand(HitResult);
}

void APlayerActor::Cmd_ChargeKi_Implementation(bool bPressed)
{
	if ( !IsControlEnable() )
		return;

	PRINTINFO();
}

void APlayerActor::Cmd_Guard_Implementation(bool bPressed)
{
	if ( !IsControlEnable() )
		return;

	PRINTINFO();
}

void APlayerActor::Cmd_Vanish_Implementation()
{
	if ( !IsControlEnable() )
		return;
	
	PRINTINFO();
}

void APlayerActor::Cmd_RushAttack_Implementation()
{
	if ( !IsControlEnable() )
		return;

	RushAttackSystem->OnAttack();
}

void APlayerActor::Cmd_EnergyBlast_Implementation()
{
	if ( !IsControlEnable() )
		return;

	PRINTINFO();
}

void APlayerActor::Cmd_Kamehameha_Implementation()
{
	if ( !IsControlEnable() )
		return;

	PRINTINFO();
}