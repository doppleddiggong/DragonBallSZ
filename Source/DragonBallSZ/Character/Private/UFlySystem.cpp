// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "UFlySystem.h"
#include "Features/UEaseFunctionLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
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

	UpstreamTick(DeltaTime);
	DownstreamTick(DeltaTime);
}

void UFlySystem::OnJump()
{
	JumpCount++;
	if (JumpCount > 3)
		return;
	
	switch (JumpCount)
	{
	case 1:
		{
			Owner->Jump();
		}
		break;

	case 2:
		{
			auto Movement = Owner->GetCharacterMovement();

			Movement->SetMovementMode( EMovementMode::MOVE_Flying );
			this->ActivateUpstream();

			Owner->bUseControllerRotationYaw = true;
			Owner->bUseControllerRotationPitch = true;
			Movement->bOrientRotationToMovement = false;
		}
		break;

	case 3:
		{
			FHitResult HitResult;
			this->OnLand(HitResult);
		}
		break;
	}
}

void UFlySystem::OnLand(const FHitResult& Hit)
{
	auto Movement = Owner->GetCharacterMovement();
	Movement->SetMovementMode( EMovementMode::MOVE_Falling );

	this->ActivateDownstream();
	
	Owner->bUseControllerRotationYaw = false;
	Owner->bUseControllerRotationPitch = false;
	Movement->bOrientRotationToMovement = true;

	JumpCount = 0;;
}


void UFlySystem::ActivateUpstream()
{
	ElapsedTime = 0.0f;
	bIsUpstream = true;

	StartLocation = Owner->GetActorLocation();
	EndLocation = StartLocation + FVector(0,0, UpstreamHeight);
}

void UFlySystem::ActivateDownstream()
{
	ElapsedTime = 0.0f;
	bIsDownstream = true;

	StartLocation = Owner->GetActorLocation();
	FVector TempEndPos = StartLocation - FVector(0,0, UpstreamHeight*3);

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

void UFlySystem::UpstreamTick(float DeltaTime)
{
	if ( !bIsUpstream )
		return;

	ElapsedTime += DeltaTime;
	
	FVector ResultLocation = UEaseFunctionLibrary::LerpVectorEase(
		StartLocation, EndLocation,
		ElapsedTime / UpstreamDuration,
		EEaseType::EaseOutQuart);

	Owner->SetActorLocation(ResultLocation, true);

	float Dist = FVector::Dist(  Owner->GetActorLocation(), EndLocation );
	if ( Dist < 10.0f || ElapsedTime > UpstreamDuration )
	{
		bIsUpstream = false;
		Owner->SetActorLocation(EndLocation, true);
	}
}

void UFlySystem::DownstreamTick(float DeltaTime)
{
	if ( !bIsDownstream )
		return;

	ElapsedTime += DeltaTime;
	
	FVector ResultLocation = UEaseFunctionLibrary::LerpVectorEase(
		StartLocation, EndLocation,
		ElapsedTime / DownstreamDuration,
		EEaseType::EaseOutQuart);

	Owner->SetActorLocation(ResultLocation, true);

	float Dist = FVector::Dist( Owner->GetActorLocation(), EndLocation );
	if ( Dist < 10.0f || ElapsedTime > DownstreamDuration )
	{
		bIsDownstream = false;
		Owner->SetActorLocation(EndLocation, true);

		Callback.ExecuteIfBound();
	}
}

