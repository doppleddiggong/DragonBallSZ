// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "AEnemyActor.h"

#include "AEnemyAIController.h"
#include "APlayerActor.h"

#include "UStatSystem.h"
#include "USightSystem.h"
#include "UEnemyFSM.h"

#include "UDBSZEventManager.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

AEnemyActor::AEnemyActor()
{
	PrimaryActorTick.bCanEverTick = true;

	StatSystem = CreateDefaultSubobject<UStatSystem>(TEXT("StatSystem"));
	SightSystem = CreateDefaultSubobject<USightSystem>(TEXT("SightSystem"));
	EnemyFSM = CreateDefaultSubobject<UEnemyFSM>(TEXT("EnemyFSM"));
	
	AutoPossessAI   = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AEnemyAIController::StaticClass();
}

void AEnemyActor::BeginPlay()
{
	Super::BeginPlay();

	if ( AActor* FoundActor = UGameplayStatics::GetActorOfClass( GetWorld(), APlayerActor::StaticClass() ) )
		TargetActor = Cast<APlayerActor>(FoundActor);

	StatSystem->InitStat(false);

	if (auto EventManager = UDBSZEventManager::Get(GetWorld()))
		EventManager->SendUpdateHealth(false, StatSystem->CurHP, StatSystem->MaxHP);

	AIEnemy = Cast<AEnemyAIController>(GetController());

	SightSystem->InitSightSystem(TargetActor, StatSystem->SightLength, StatSystem->SightAngle );
	SightSystem->OnSightDetect.AddDynamic(this, &AEnemyActor::OnSightDetect);

	// TODO, 할것 있으면 하세요.
	// EnemyFSM->
}

void AEnemyActor::OnSightDetect(bool Target)
{
	AIEnemy->SetTarget(Target ? TargetActor : nullptr );
}

void AEnemyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if ( !IsValid(TargetActor ))
		return;

	if ( !IsHit )
	{
		// 피격이거나, 뭔가 안봐야 할경우에는 안해야 한다
		auto TargetRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargetActor->GetActorLocation() );
		this->SetActorRotation( UKismetMathLibrary::RInterpTo(GetActorRotation(), TargetRot, DeltaTime, RotateLerpSpeed) );
	}
}