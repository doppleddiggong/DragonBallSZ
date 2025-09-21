// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "APlayerActor.h"

// CombatCharacter Shared
#include "UStatSystem.h"
#include "UHitStopSystem.h"
#include "URushAttackSystem.h"
#include "UKnockbackSystem.h"
#include "UDashSystem.h"
#include "UFlySystem.h"
#include "UCharacterData.h"
#include "UChargeKiSystem.h"

// PlayerActor Only
#include "AEnemyActor.h"
#include "UCameraShakeSystem.h"

// Shared
#include "Core/Macro.h"
#include "DragonBallSZ.h"

#include "UDBSZEventManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

#define GOKU_DATA	TEXT("/Game/CustomContents/MasterData/Goku_Data.Goku_Data")

APlayerActor::APlayerActor()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraShakeSystem = CreateDefaultSubobject<UCameraShakeSystem>(TEXT("CameraShakeSystem"));

	static ConstructorHelpers::FObjectFinder<UCharacterData> CD( GOKU_DATA );
	if (CD.Succeeded())
		CharacterData = CD.Object;
}

void APlayerActor::BeginPlay()
{
	Super::BeginPlay();

	// PlayerActor Only
	if ( AActor* FoundActor = UGameplayStatics::GetActorOfClass( GetWorld(), AEnemyActor::StaticClass() ) )
		TargetActor = Cast<AEnemyActor>(FoundActor);

	// AsyncLoad
	CharacterData->LoadHitMontage(HitMontages);
	CharacterData->LoadDeathMontage(DeathMontage);
	CharacterData->LoadBlastMontage(BlastMontages);
	CharacterData->LoadChargeKiMontage(ChargeKiMontage);
	CharacterData->LoadKamehameMontage(KamehameMontage);
	CharacterData->LoadIntroMontage(IntroMontage);
	CharacterData->LoadWinMontage(WinMontage);

	CharacterData->LoadDashVFX(DashVFX);
	CharacterData->LoadChargeKiVFX(ChargeKiVFX);

	CharacterData->LoadEnergyBlast(EnergyBlastFactory);
	CharacterData->LoadKamehame(KamehamehaFactory);
	
	CameraShakeSystem->InitSystem(this);	

	// ActorComponent 초기화
	StatSystem->InitStat(true, ECharacterType::Songoku);
	RushAttackSystem->InitSystem(this, CharacterData);
	KnockbackSystem->InitSystem(this);
	DashSystem->InitSystem(this, DashVFX);
	FlySystem->InitSystem(this, BIND_DYNAMIC_DELEGATE(FEndCallback, this, APlayerActor, OnFlyEnd));
	HitStopSystem->InitSystem(this);
	ChargeKiSystem ->InitSystem(this, ChargeKiVFX);

	// 이벤트 매니저를 통한 이벤트 등록 및 제어
	EventManager = UDBSZEventManager::Get(GetWorld());
	EventManager->OnDash.AddDynamic(this, &APlayerActor::OnDash);
	EventManager->OnTeleport.AddDynamic(this, &APlayerActor::OnTeleport);
	EventManager->OnAttack.AddDynamic(this, &APlayerActor::OnAttack);
	EventManager->OnSpecialAttack.AddDynamic(this, &APlayerActor::OnSpecialAttack);
	EventManager->OnGuard.AddDynamic(this, &APlayerActor::OnGuard);
	EventManager->OnAvoid.AddDynamic(this, &APlayerActor::OnAvoid);
}

void APlayerActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (!EventManager) return;
	
	EventManager->OnDash.RemoveDynamic(this, &APlayerActor::OnDash);
	EventManager->OnTeleport.RemoveDynamic(this, &APlayerActor::OnTeleport);
	EventManager->OnAttack.RemoveDynamic(this, &APlayerActor::OnAttack);
	EventManager->OnSpecialAttack.RemoveDynamic(this, &APlayerActor::OnSpecialAttack);
	EventManager->OnGuard.RemoveDynamic(this, &APlayerActor::OnGuard);
	EventManager->OnAvoid.RemoveDynamic(this, &APlayerActor::OnAvoid);
}

void APlayerActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if ( RushAttackSystem->ShouldLookAtTarget())
		this->OnLookTarget();
}

void APlayerActor::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (FlySystem)
		FlySystem->OnLand(Hit);
}

void APlayerActor::OnRestoreAvoid()
{
	EventManager->SendAvoid(this, false);
}

void APlayerActor::OnDash(AActor* Target, bool IsDashing, FVector Direction)
{
	if ( this != Target )
		return;
}

void APlayerActor::OnTeleport(AActor* Target)
{
	if ( this != Target )
		return;
}

void APlayerActor::OnAttack(AActor* Target, int ComboCount)
{
	if ( this != Target )
		return;
}

void APlayerActor::OnSpecialAttack(AActor* Target, int32 SpecialIndex)
{
	if ( this != Target )
		return;
}

void APlayerActor::OnGuard(AActor* Target, bool bState)
{
	if ( this != Target )
		return;
}

void APlayerActor::OnAvoid(AActor* Target, bool bState)
{
	if ( this != Target )
		return;
}

void APlayerActor::Cmd_Move_Implementation(const FVector2D& Axis)
{
	if ( !IsMoveEnable() )
		return;
	
	const FRotator ActorRot = GetActorRotation();
	
	if ( MoveComp->MovementMode == MOVE_Walking || MoveComp->MovementMode == MOVE_Falling )
	{
		// Right : XZ
		// Forward : Z
		const FRotator YawOnlyRot(0.0f, ActorRot.Yaw, 0.0f);
		const FRotator YawWithRollRot(ActorRot.Roll, ActorRot.Yaw, 0.0f); // Roll은 시각 효과용일 수 있음

		const FVector RightDir   = FRotationMatrix(YawWithRollRot).GetUnitAxis(EAxis::Y);
		const FVector ForwardDir = FRotationMatrix(YawOnlyRot).GetUnitAxis(EAxis::X);

		AddMovementInput(RightDir, Axis.X);
		AddMovementInput(ForwardDir, Axis.Y);
	}
	else if ( MoveComp->MovementMode == MOVE_Flying )
	{
		// Right : YZ
		// Forward : YZ
		// 공중 이동: Pitch + Yaw 기준 3D 방향
		const FRotator FullRot(ActorRot.Pitch, ActorRot.Yaw, 0.0f );

		const FVector RightDir   = FRotationMatrix(FullRot).GetUnitAxis(EAxis::Y);
		const FVector ForwardDir = FRotationMatrix(FullRot).GetUnitAxis(EAxis::X);

		AddMovementInput(RightDir, Axis.X);
		AddMovementInput(ForwardDir, Axis.Y);
	}
}

void APlayerActor::Cmd_Look_Implementation(const FVector2D& Axis)
{
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void APlayerActor::Cmd_AltitudeUp_Implementation()
{
	if ( !IsMoveEnable() )
		return;

	FlySystem->OnAltitudePress(true);
}

void APlayerActor::Cmd_AltitudeDown_Implementation()
{
	if ( !IsMoveEnable() )
		return;

	FlySystem->OnAltitudePress(false);
}

void APlayerActor::Cmd_AltitudeReleased_Implementation()
{
	if ( !IsMoveEnable() )
		return;

	FlySystem->OnAltitudeRelease();
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

	ChargeKiSystem->ActivateEffect(bPressed);
}

void APlayerActor::Cmd_Guard_Implementation(bool bPressed)
{
	if ( !IsControlEnable() )
		return;

	EventManager->SendGuard(this, bPressed);
}

void APlayerActor::Cmd_Vanish_Implementation()
{
	if ( !IsControlEnable() )
		return;
	
	EventManager->SendAvoid(this, true);

	GetWorld()->GetTimerManager().ClearTimer(AvoidTimer);
	GetWorld()->GetTimerManager().SetTimer(
		AvoidTimer, this, &APlayerActor::OnRestoreAvoid, AvoidTime, false
	);
}

void APlayerActor::Cmd_RushAttack_Implementation()
{
	if ( !IsControlEnable() )
		return;

	RushAttackSystem->OnAttack();
}

void APlayerActor::Cmd_EnergyBlast_Implementation()
{
	if (!IsBlastShootEnable() )
		return;
	
	this->EnergyBlastShoot();
}

void APlayerActor::Cmd_Kamehameha_Implementation()
{
	if ( !IsKamehameEnable() )
		return;
	
	this->KamehameShoot();
}