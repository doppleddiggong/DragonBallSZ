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

// PlayerActor Only
#include "AEnemyActor.h"
#include "UCameraShakeSystem.h"

// Shared
#include "Core/Macro.h"
#include "DragonBallSZ.h"
#include "EnergyBlastActor.h"
#include "UDBSZEventManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

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
	CharacterData->LoadDashVFX(DashVFX);
	CharacterData->LoadEnergyBlast(EnergyBlastFactory);
	
	CameraShakeSystem->InitSystem(this);	

	// ActorComponent 초기화
	StatSystem->InitStat(true);
	RushAttackSystem->InitSystem(this, CharacterData);
	RushAttackSystem->SetDamage( StatSystem->Damage );
	KnockbackSystem->InitSystem(this);
	DashSystem->InitSystem(this, DashVFX);
	FlySystem->InitSystem(this, BIND_DYNAMIC_DELEGATE(FEndCallback, this, APlayerActor, OnFlyEnd));
	HitStopSystem->InitSystem(this);

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
}

void APlayerActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if ( RushAttackSystem->ShouldLookAtTarget())
		this->OnLookTarget();

	// 에너지탄 재장전 로직
	if (RemainBlastShot < MaxRepeatBlastShot)
	{
		BlastShotRechargeTime += DeltaTime;
		if (BlastShotRechargeTime >= BlastShotRechargeDuration)
		{
			RemainBlastShot = MaxRepeatBlastShot;
			BlastShotRechargeTime = 0.0f;
			PRINT_STRING(TEXT("Energy Blast Recharged"));
		}
	}
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
	const TCHAR* PrintMsg = IsDashing ? TEXT("Player Dashing Start") : TEXT("Player Dashing Complete");
	PRINTLOG(TEXT("%s"), PrintMsg);
}

void APlayerActor::OnTeleport(AActor* Target)
{
	if ( this != Target )
		return;

	PRINTLOG(TEXT("OnTeleport"));
}

void APlayerActor::OnAttack(AActor* Target, int ComboCount)
{
	if ( this != Target )
		return;

	PRINTLOG(TEXT("ComboCount : %d"), ComboCount);
}

void APlayerActor::OnSpecialAttack(AActor* Target, int32 SpecialIndex)
{
	if ( this != Target )
		return;

	PRINTLOG(TEXT("OnSpecialAttack : %d"), SpecialIndex);
}

void APlayerActor::OnGuard(AActor* Target, bool bState)
{
	if ( this != Target )
		return;
	
	const TCHAR* PrintMsg = bState ? TEXT("Player Guard Start") : TEXT("Player Guard End");
	PRINTLOG(TEXT("%s"), PrintMsg);
}

void APlayerActor::OnAvoid(AActor* Target, bool bState)
{
	if ( this != Target )
		return;
	
	const TCHAR* PrintMsg = bState ? TEXT("Player Avoid Start") : TEXT("Player Avoid End");
	PRINTLOG(TEXT("%s"), PrintMsg);
}

void APlayerActor::OnPowerCharge(AActor* Target, bool bState)
{
	if ( this != Target )
		return;
	
	const TCHAR* PrintMsg = bState ? TEXT("Player PowerCharge Start") : TEXT("Player PowerCharge End");
	PRINTLOG(TEXT("%s"), PrintMsg);
}

void APlayerActor::Cmd_Move_Implementation(const FVector2D& Axis)
{
	if ( !IsMoveEnable() )
		return;

	// const FRotator ActorRot = GetActorRotation();
	//
	// auto MoveComp = GetCharacterMovement();
	// if ( MoveComp->MovementMode == MOVE_Walking || MoveComp->MovementMode == MOVE_Falling )
	// {
	// 	// Right : XZ
	// 	// Forward : Z
	// 	UKismetMathLibrary::GetRightVector(FRotator(ActorRot.Roll, 0.0f, ActorRot.Yaw));
	// 	UKismetMathLibrary::GetForwardVector(FRotator(0.0f, 0.0f, ActorRot.Yaw));
	// }
	// else if ( MoveComp->MovementMode == MOVE_Flying )
	// {
	// 	// Right : YZ
	// 	// Forward : YZ
	// 	UKismetMathLibrary::GetRightVector(FRotator(0.0, ActorRot.Pitch, ActorRot.Yaw));
	// 	UKismetMathLibrary::GetForwardVector(FRotator(0.0f, ActorRot.Pitch, ActorRot.Yaw));
	// }

	
	
	// const FRotator YawOnly(0.f, ActorRot.Yaw, 0.f);
	// const FVector Fwd_Yaw   = FRotationMatrix(YawOnly).GetUnitAxis(EAxis::X);
	// const FVector Right_Yaw = FRotationMatrix(YawOnly).GetUnitAxis(EAxis::Y);
	//
	// // 모드별 방향 선택
	// FVector FwdDir  = FVector::ZeroVector;
	// FVector RightDir= FVector::ZeroVector;
	//
	// switch (Move->MovementMode)
	// {
	// case MOVE_Flying:
	// 	FwdDir   = GetActorForwardVector(); // 피치 포함
	// 	RightDir = GetActorRightVector();
	// 	break;
	//
	// case MOVE_Walking:
	// case MOVE_NavWalking:
	// case MOVE_Falling:
	// case MOVE_Swimming:
	// default:
	// 	FwdDir   = Fwd_Yaw;                 // 피치 제거
	// 	RightDir = Right_Yaw;
	// 	break;
	// }
	//
	// // 좌우
	// if (FMath::Abs(AxisY) > KINDA_SMALL_NUMBER && !RightDir.IsNearlyZero())
	// {
	// 	AddMovementInput(RightDir, AxisY);
	// }
	//
	// // 전후
	// if (FMath::Abs(AxisX) > KINDA_SMALL_NUMBER && !FwdDir.IsNearlyZero())
	// {
	// 	AddMovementInput(FwdDir, AxisX);
	// }
	//
	// // 수직(입력이 있을 때만, 비행/수영에서만)
	// if (FMath::Abs(AxisZ) > KINDA_SMALL_NUMBER &&
	// 	(Move->MovementMode == MOVE_Flying || Move->MovementMode == MOVE_Swimming))
	// {
	// 	AddMovementInput(FVector::UpVector, AxisZ);
	// }
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

	EventManager->SendPowerCharge(this, bPressed);
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
	if ( !IsControlEnable() )
		return;

	// 발사 딜레이 체크
	if (GetWorld()->GetTimeSeconds() < LastBlastShotTime + BlastShotDelay)
		return;

	// 잔탄 체크
	if (RemainBlastShot > 0)
	{
		FActorSpawnParameters Params;
		Params.Owner = this;
		Params.Instigator = this;

		GetWorld()->SpawnActor<AEnergyBlastActor>(
			EnergyBlastFactory,
			this->GetActorTransform(),
			Params
		);

		// 발사 처리
		RemainBlastShot--;
		LastBlastShotTime = GetWorld()->GetTimeSeconds();
		BlastShotRechargeTime = 0.0f;

		EventManager->SendCameraShake(this, EAttackPowerType::Normal );
		
		PRINT_STRING( TEXT("Energy Blast Fired! %d / %d"), RemainBlastShot, MaxRepeatBlastShot);
	}
	else
	{
		PRINT_STRING(TEXT("Out of Energy Blast"));
	}
}

void APlayerActor::Cmd_Kamehameha_Implementation()
{
	if ( !IsControlEnable() )
		return;

	EventManager->SendSpecialAttack(this, 1);
}