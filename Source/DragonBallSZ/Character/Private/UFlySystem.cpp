// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "UFlySystem.h"

#include "UDBSZEventManager.h"
#include "ACombatCharacter.h"

#include "Components/CapsuleComponent.h"
#include "Shared/FEaseHelper.h"
#include "Features/UEaseFunctionLibrary.h"

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
void UFlySystem::InitSystem(class ACombatCharacter* InOwner, FEndCallback InCallback)
{
	this->Owner = InOwner;
	this->Callback = InCallback;

	EventManager = UDBSZEventManager::Get(GetWorld());
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
			this->ActivateUpstream();

			Owner->SetFlying();
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
	Owner->SetFallingToWalk();
	this->ActivateDownstream();

	JumpCount = 0;
}


void UFlySystem::ActivateUpstream()
{
	ElapsedTime = 0.0f;
	bIsUpstream = true;

	StartLocation = Owner->GetActorLocation();
	EndLocation = StartLocation + FVector(0,0, UpstreamHeight);

	EventManager->SendUpstream(Owner, true);
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
		{
			const float CapsuleHalfHeight = Owner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
			EndLocation = HitInfo.Location + FVector(0, 0, CapsuleHalfHeight);
		}
	else
	{
		EndLocation = TempEndPos;
	}

	EventManager->SendDownstream(Owner, true);
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
	if ( Dist < AlmostDist || ElapsedTime > UpstreamDuration )
	{
		bIsUpstream = false;
		Owner->SetActorLocation(EndLocation, true);

		EventManager->SendUpstream(Owner, false);
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
	if ( Dist < AlmostDist || ElapsedTime > DownstreamDuration )
	{
		bIsDownstream = false;
		Owner->SetActorLocation(EndLocation, true);

		EventManager->SendDownstream(Owner, false);
		
		Callback.ExecuteIfBound();
	}
}