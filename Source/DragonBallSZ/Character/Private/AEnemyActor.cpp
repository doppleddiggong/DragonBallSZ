// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "AEnemyActor.h"

#include "AEnemyAIController.h"
#include "APlayerActor.h"
#include "UStatSystem.h"
#include "UDBSZEventManager.h"
#include "USightSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

AEnemyActor::AEnemyActor()
{
	PrimaryActorTick.bCanEverTick = true;

	StatSystem = CreateDefaultSubobject<UStatSystem>(TEXT("StatSystem"));
	SightSystem = CreateDefaultSubobject<USightSystem>(TEXT("SightSystem"));

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

	auto TargetRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargetActor->GetActorLocation() );
	this->SetActorRotation( UKismetMathLibrary::RInterpTo(GetActorRotation(), TargetRot, DeltaTime, RotateLerpSpeed) );
}