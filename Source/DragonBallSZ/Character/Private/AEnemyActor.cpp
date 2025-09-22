// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "AEnemyActor.h"

// CombatCharacter Shared
#include "UStatSystem.h"
#include "UHitStopSystem.h"
#include "URushAttackSystem.h"
#include "UKnockbackSystem.h"
#include "UDashSystem.h"
#include "UFlySystem.h"
#include "UChargeKiSystem.h"
#include "ACombatLevelScript.h"
#include "ADBSZGameMode.h"

// EnemyActor Only
#include "APlayerActor.h"
#include "UEnemyFSM.h"
#include "USightSystem.h"

// Shared
#include "Core/Macro.h"
#include "UDBSZEventManager.h"
#include "Kismet/GameplayStatics.h"

AEnemyActor::AEnemyActor()
{
	PrimaryActorTick.bCanEverTick = true;

	EnemyFSM			= CreateDefaultSubobject<UEnemyFSM>(TEXT("EnemyFSM"));
	SightSystem			= CreateDefaultSubobject<USightSystem>(TEXT("SightSystem"));
}

void AEnemyActor::BeginPlay()
{
	Super::BeginPlay();

	// EnemyActor Only
	if ( AActor* Player = UGameplayStatics::GetActorOfClass( GetWorld(), APlayerActor::StaticClass() ) )
		TargetActor = Cast<APlayerActor>(Player);

	auto GameMode = Cast<ADBSZGameMode>(UGameplayStatics::GetGameMode(this));
	this->SetupCharacterFromType(GameMode->EnemyType, GameMode->bIsEnemyAnother);

	SightSystem->InitSightSystem(TargetActor, StatSystem->GetSightLength(), StatSystem->GetSightAngle() );
	SightSystem->OnSightDetect.AddDynamic(this, &AEnemyActor::OnSightDetect);	
	
	// ActorComponent 초기화
	StatSystem->InitStat(false, CharacterType);
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
	// AIEnemy->SetTarget(Target ? TargetActor : nullptr );
}

void AEnemyActor::OnRestoreAvoid()
{
	EventManager->SendAvoid(this, false);
}