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
#include "UChargeKiSystem.h"

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

    if (!IsValid(CharacterData))
    {
        PRINTLOG(TEXT("AEnemyActor::BeginPlay: CharacterData is invalid. Cannot load character assets."));
        // Handle gracefully, e.g., return or use default values
        return;
    }

	// EnemyActor Only
	if ( AActor* Player = UGameplayStatics::GetActorOfClass( GetWorld(), APlayerActor::StaticClass() ) )
		TargetActor = Cast<APlayerActor>(Player);
	
	AIEnemy = Cast<AEnemyAIController>(GetController());
	SightSystem->InitSightSystem(TargetActor, StatSystem->SightLength, StatSystem->SightAngle );
	SightSystem->OnSightDetect.AddDynamic(this, &AEnemyActor::OnSightDetect);	

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

	// ActorComponent 초기화
	StatSystem->InitStat(false);
	RushAttackSystem->InitSystem(this, CharacterData);
	KnockbackSystem->InitSystem(this);
	DashSystem->InitSystem(this, DashVFX);
	FlySystem->InitSystem(this, BIND_DYNAMIC_DELEGATE(FEndCallback, this, AEnemyActor, OnFlyEnd));
	HitStopSystem->InitSystem(this);
	ChargeKiSystem ->InitSystem(this, ChargeKiVFX);

	// 이벤트 매니저를 통한 이벤트 등록 및 제어
	EventManager = UDBSZEventManager::Get(GetWorld());
}

void AEnemyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (this->IsCombatStart() == false ||
		this->IsCombatResult())
	{
		// 전투 시작전
		// 전투 결과후
		return;
	}

	// 연출적 고정 시점에는 Pass
	if ( IsHolding())
		return;
	
	if ( !IsHit || !RushAttackSystem->bIsDashing )
		this->OnLookTarget();
	else if ( RushAttackSystem->ShouldLookAtTarget() )
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