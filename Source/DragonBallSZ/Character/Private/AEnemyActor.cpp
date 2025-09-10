// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "AEnemyActor.h"

#include "AEnemyAIController.h"
#include "APlayerActor.h"

#include "UStatSystem.h"
#include "USightSystem.h"
#include "UEnemyFSM.h"

#include "UDBSZEventManager.h"
#include "UHitStopSystem.h"
#include "Components/ArrowComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

AEnemyActor::AEnemyActor()
{
	PrimaryActorTick.bCanEverTick = true;

	StatSystem			= CreateDefaultSubobject<UStatSystem>(TEXT("StatSystem"));
	HitStopSystem		= CreateDefaultSubobject<UHitStopSystem>(TEXT("HitStopSystem"));
	SightSystem			= CreateDefaultSubobject<USightSystem>(TEXT("SightSystem"));

	EnemyFSM			= CreateDefaultSubobject<UEnemyFSM>(TEXT("EnemyFSM"));
	
	AutoPossessAI   = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AEnemyAIController::StaticClass();

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
}

void AEnemyActor::BeginPlay()
{
	Super::BeginPlay();

	if ( AActor* Player = UGameplayStatics::GetActorOfClass( GetWorld(), APlayerActor::StaticClass() ) )
		TargetActor = Cast<APlayerActor>(Player);

	StatSystem->InitStat(false);

	if (auto EventManager = UDBSZEventManager::Get(GetWorld()))
		EventManager->SendUpdateHealth(false, StatSystem->CurHP, StatSystem->MaxHP);

	AIEnemy = Cast<AEnemyAIController>(GetController());

	SightSystem->InitSightSystem(TargetActor, StatSystem->SightLength, StatSystem->SightAngle );
	SightSystem->OnSightDetect.AddDynamic(this, &AEnemyActor::OnSightDetect);

	HitStopSystem->InitSystem(this);
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

void AEnemyActor::GetBodyLocation(USceneComponent* SceneComp, FVector& OutStart, FVector& OutEnd) const
{
	if (!SceneComp)	
	{
		OutStart = FVector::ZeroVector;
		OutEnd   = FVector::ZeroVector;
		return;
	}

	OutStart = SceneComp->GetComponentLocation();
	OutEnd = OutStart + SceneComp->GetForwardVector() * TraceLength;
}
