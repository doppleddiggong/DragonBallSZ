// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "UFlySystem.h"
#include "Features/UEaseFunctionLibrary.h"
#include "Shared/FEaseHelper.h"

UFlySystem::UFlySystem()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UFlySystem::BeginPlay()
{
	Super::BeginPlay();
}

void UFlySystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UFlySystem::ActivateFlyProcess()
{
	ElapsedTime = 0.0f;
	FlyingProcess = true;

	StartLocation = Owner->GetActorLocation();
	EndLocation = StartLocation + FVector(0,0, FlyHeight);
}

void UFlySystem::ActivateLandingProcess()
{
	ElapsedTime = 0.0f;
	LandingProcess = true;

	StartLocation = Owner->GetActorLocation();
	FVector TempEndPos = StartLocation - FVector(0,0, FlyHeight*3);

	FHitResult HitInfo;

	FCollisionQueryParams CollisionParam;
	CollisionParam.AddIgnoredActor(Owner);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitInfo,
		StartLocation, TempEndPos,
		ECC_Visibility, CollisionParam);

	if ( bHit)
		EndLocation = HitInfo.Location;
}

void UFlySystem::FlyTick(float DeltaTime)
{
	if ( !FlyingProcess )
		return;

	ElapsedTime += DeltaTime;
	
	FVector Result = UEaseFunctionLibrary::LerpVectorEase(
		StartLocation, EndLocation,
		ElapsedTime / FlyDuration,
		EEaseType::EaseOutQuart);	
}

void UFlySystem::LandingTick(float DeltaTime)
{
	if ( !LandingProcess )
		return;
}

