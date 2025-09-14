// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "AEnemyActor.h"

// CombatCharacter Shared
#include "UStatSystem.h"
#include "UHitStopSystem.h"
#include "URushAttackSystem.h"
#include "UKnockbackSystem.h"
#include "UDashSystem.h"
#include "UFlySystem.h"
#include "UCharacterData.h"

// EnemyActor Only
#include "AEnemyAIController.h"
#include "APlayerActor.h"
#include "UEnemyFSM.h"
#include "USightSystem.h"

// Shared
#include "Core/Macro.h"
#include "DragonBallSZ.h"
#include "UDBSZEventManager.h"
#include "Kismet/GameplayStatics.h"

#define VEGE_DATA	TEXT("/Game/CustomContents/MasterData/Vege_Data.Vege_Data")


AEnemyActor::AEnemyActor()
{
	PrimaryActorTick.bCanEverTick = true;

	EnemyFSM			= CreateDefaultSubobject<UEnemyFSM>(TEXT("EnemyFSM"));
	SightSystem			= CreateDefaultSubobject<USightSystem>(TEXT("SightSystem"));

	AutoPossessAI   = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AEnemyAIController::StaticClass();

	{
		static ConstructorHelpers::FObjectFinder<UCharacterData> CD( VEGE_DATA );
		if (CD.Succeeded())
			CharacterData = CD.Object;
	}
}

void AEnemyActor::BeginPlay()
{
	Super::BeginPlay();

	// EnemyActor Only
	if ( AActor* Player = UGameplayStatics::GetActorOfClass( GetWorld(), APlayerActor::StaticClass() ) )
		TargetActor = Cast<APlayerActor>(Player);
	
	AIEnemy = Cast<AEnemyAIController>(GetController());
	SightSystem->InitSightSystem(TargetActor, StatSystem->SightLength, StatSystem->SightAngle );
	SightSystem->OnSightDetect.AddDynamic(this, &AEnemyActor::OnSightDetect);	

	// AsyncLoad
	CharacterData->LoadHitMontage(HitMontages);
	CharacterData->LoadDeathMontage(DeathMontage);
	CharacterData->LoadDashVFX(DashVFX);
	CharacterData->LoadEnergyBlast(EnergyBlastFactory);

	// ActorComponent 초기화
	StatSystem->InitStat(false);
	RushAttackSystem->InitSystem(this, CharacterData);
	RushAttackSystem->SetDamage( StatSystem->Damage );
	KnockbackSystem->InitSystem(this);
	DashSystem->InitSystem(this, DashVFX);
	FlySystem->InitSystem(this, BIND_DYNAMIC_DELEGATE(FEndCallback, this, AEnemyActor, OnFlyEnd));
	HitStopSystem->InitSystem(this);

	// 이벤트 매니저를 통한 이벤트 등록 및 제어
	EventManager = UDBSZEventManager::Get(GetWorld());
	EventManager->OnDash.AddDynamic(this, &AEnemyActor::OnDash);
	EventManager->OnTeleport.AddDynamic(this, &AEnemyActor::OnTeleport);
	EventManager->OnAttack.AddDynamic(this, &AEnemyActor::OnAttack);
	EventManager->OnSpecialAttack.AddDynamic(this, &AEnemyActor::OnSpecialAttack);
	EventManager->OnGuard.AddDynamic(this, &AEnemyActor::OnGuard);
	EventManager->OnAvoid.AddDynamic(this, &AEnemyActor::OnAvoid);
	EventManager->OnPowerCharge.AddDynamic(this, &AEnemyActor::OnPowerCharge);
	EventManager->SendUpdateHealth(true, StatSystem->CurHP, StatSystem->MaxHP);
}

void AEnemyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if ( !IsHit )
	{
		this->OnLookTarget();
	}
	else if ( RushAttackSystem->ShouldLookAtTarget())
		this->OnLookTarget();
}

void AEnemyActor::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (FlySystem)
		FlySystem->OnLand(Hit);
}

void AEnemyActor::OnSightDetect(bool Target)
{
	AIEnemy->SetTarget(Target ? TargetActor : nullptr );
}

void AEnemyActor::OnRestoreAvoid()
{
	EventManager->SendAvoid(this, false);
}

void AEnemyActor::OnDash(AActor* Target, bool IsDashing, FVector Direction )
{
	if ( this != Target )
		return;
	const TCHAR* PrintMsg = IsDashing ? TEXT("Enemy Dashing Start") : TEXT("Enemy Dashing Complete");
	PRINTLOG(TEXT("%s"), PrintMsg);
}

void AEnemyActor::OnTeleport(AActor* Target)
{
	if ( this != Target )
		return;

	PRINTLOG(TEXT("Enemy OnTeleport"));
}

void AEnemyActor::OnAttack(AActor* Target, int ComboCount)
{
	if ( this != Target )
		return;

	PRINTLOG(TEXT("Enemy ComboCount : %d"), ComboCount);
}

void AEnemyActor::OnSpecialAttack(AActor* Target, int32 SpecialIndex)
{
	if ( this != Target )
		return;

	PRINTLOG(TEXT("Enemy OnSpecialAttack : %d"), SpecialIndex);
}

void AEnemyActor::OnGuard(AActor* Target, bool bState)
{
	if ( this != Target )
		return;
	
	const TCHAR* PrintMsg = bState ? TEXT("Enemy Guard Start") : TEXT("Enemy Guard End");
	PRINTLOG(TEXT("%s"), PrintMsg);
}

void AEnemyActor::OnAvoid(AActor* Target, bool bState)
{
	if ( this != Target )
		return;
	
	const TCHAR* PrintMsg = bState ? TEXT("Enemy Avoid Start") : TEXT("Enemy Avoid End");
	PRINTLOG(TEXT("%s"), PrintMsg);
}

void AEnemyActor::OnPowerCharge(AActor* Target, bool bState)
{
	if ( this != Target )
		return;
	
	const TCHAR* PrintMsg = bState ? TEXT("Enemy PowerCharge Start") : TEXT("Enemy PowerCharge End");
	PRINTLOG(TEXT("%s"), PrintMsg);
}