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
#include "AEnergyBlastActor.h"
#include "KamehamehaActor.h"

#include "EAnimMontageType.h"
#include "UDBSZEventManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

#define GOKU_DATA	TEXT("/Game/CustomContents/MasterData/Goku_Data.Goku_Data")

APlayerActor::APlayerActor()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraShakeSystem = CreateDefaultSubobject<UCameraShakeSystem>(TEXT("CameraShakeSystem"));

	{
		static ConstructorHelpers::FObjectFinder<UCharacterData> CD( GOKU_DATA );
		if (CD.Succeeded())
			CharacterData = CD.Object;
	}

	bUseControllerRotationYaw = true;
	if (auto* Movecomp = GetCharacterMovement())
	{
		Movecomp->bOrientRotationToMovement = false;

		// 플라잉 모드 미끄러짐 방지 설정
		// 높은 값으로 설정하여 즉시 멈추도록 함
		Movecomp->BrakingDecelerationFlying = 4096.0f; // 기본값 0.0f
    
		// 추가적으로 마찰력도 조정 가능
		Movecomp->BrakingFriction = 4.0f; // 기본값 0.0f
	}
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
	StatSystem->InitStat(true);
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
	EventManager->OnPowerCharge.AddDynamic(this, &APlayerActor::OnPowerCharge);
	EventManager->SendUpdateHealth(true, StatSystem->CurHP, StatSystem->MaxHP);
	EventManager->SendUpdateKi(true, StatSystem->CurKi, StatSystem->MaxKi);
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
	EventManager->OnPowerCharge.RemoveDynamic(this, &APlayerActor::OnPowerCharge);
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

void APlayerActor::OnPowerCharge(AActor* Target, bool bState)
{
	if ( this != Target )
		return;
}

void APlayerActor::Cmd_Move_Implementation(const FVector2D& Axis)
{
	if ( !IsMoveEnable() )
		return;
	
	const FRotator ActorRot = GetActorRotation();
	
	auto MoveComp = GetCharacterMovement();
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
		const FRotator FullRot(0.0f, ActorRot.Pitch, ActorRot.Yaw);  // Roll은 보통 생략
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
	
	EventManager->SendCameraShake(this, EAttackPowerType::Small );
	
	this->PlaySoundAttack();
	
	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this;

	this->UseBlast();
	this->PlayTypeMontage(EAnimMontageType::Blast);
	LastBlastShotTime = GetWorld()->GetTimeSeconds();

	GetWorld()->SpawnActor<AEnergyBlastActor>(
		EnergyBlastFactory,
		this->GetActorTransform(),
		Params
	);
}

void APlayerActor::Cmd_Kamehameha_Implementation()
{
	if ( !IsKamehameEnable() )
		return;
	
	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this;
	
	auto KamehameActor = GetWorld()->SpawnActor<AKamehamehaActor>(
		KamehamehaFactory,
		this->GetActorTransform(),
		Params
	);

	KamehameActor->StartKamehame(this, TargetActor);
}