// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "USightSystem.h"
#include "GameColor.h"
#include "Kismet/KismetMathLibrary.h"

USightSystem::USightSystem()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void USightSystem::BeginPlay()
{
	Super::BeginPlay();
}

void USightSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if ( !IsValid(TargetActor) || !IsValid(OwnerActor) )
		return;

	this->SightLineTrace(SightLength, SightAngle);

	if ( DetectSightLength(SightLength) && DetectSightAngle(SightAngle) )
	{
		// 탐지된 상태
		if ( DetectTarget )
		{
			// 현재 탐지 상태값과 동일함
			this->AddElapsedTime();
			// NOTE. 탐지된 시간에 비례해서 무엇인가 하고 싶다면..
		}
		else
		{
			// 현재 탐지 상태값과 다름
			DetectTarget = true;
			ElapsedTime = 0.0f;

			OnSightDetect.Broadcast(DetectTarget);
		}
	}
	else
	{
		// 탐지되지 않은 상태
		if ( DetectTarget )
		{
			// 현재 탐지 상태값과 다름
			DetectTarget = false;
			ElapsedTime = 0.0f;

			OnSightDetect.Broadcast(DetectTarget);
		}
		else
		{
			// 현재 탐지 상태값과 동일함
			this->AddElapsedTime();
			
			// NOTE. 탐지되지않은 시간에 비례해서 무엇인가 하고 싶다면..
		}
	}
}

void USightSystem::InitSightSystem(AActor* InTargetActor, const float InSightLength, const float InSightAngle)
{
	this->OwnerActor = GetOwner();
	this->TargetActor = InTargetActor;
	this->SightLength = InSightLength;
	this->SightAngle = InSightAngle;
}

void USightSystem::SightLineTrace(float InLength, float InAngle) const
{
	if ( !bDrawDebugLine )
		return;
	
	UWorld* OwnerWorld = OwnerActor->GetWorld();
	auto LineColor = DetectTarget ? GameColor::Pink : GameColor::Green;

	{
		{
			const auto Start = OwnerActor->GetActorLocation();
			const auto EndDirection = OwnerActor->GetActorForwardVector();
			const auto End = Start + EndDirection * InLength;
			
			DrawDebugLine(
				OwnerWorld, Start, End, LineColor,
				false,
				0.0f,
				0,
				1.0f );
		}
		
		{
			// Left
			const auto Start = OwnerActor->GetActorLocation();
			const FRotator YawRotation(0.f, -InAngle, 0.f);   // -각도만큼 Yaw 회전
			const FVector EndDirection = OwnerActor->GetActorForwardVector().RotateAngleAxis(YawRotation.Yaw, FVector::UpVector);
			auto End = Start + EndDirection * InLength;

			DrawDebugLine(
				OwnerWorld, Start, End, LineColor,
				false,
				0.0f,
				0,
				1.0f );
		}

		{
			// Right
			const auto Start = OwnerActor->GetActorLocation();
			const FRotator YawRotation(0.f, InAngle, 0.f);   // -각도만큼 Yaw 회전
			const FVector EndDirection = OwnerActor->GetActorForwardVector().RotateAngleAxis(YawRotation.Yaw, FVector::UpVector);
			auto End = Start + EndDirection * InLength;

			DrawDebugLine(
				OwnerWorld, Start, End, LineColor,
				false,
				0.0f,
				0,
				1.0f );
		}
	}
}

bool USightSystem::DetectSightLength(float InLength) const
{
	const float Dist = FVector::Dist( TargetActor->GetActorLocation(), OwnerActor->GetActorLocation());
	return Dist < InLength;
}

bool USightSystem::DetectSightAngle(float InAngle) const
{
	auto Direction = TargetActor->GetActorLocation() - OwnerActor->GetActorLocation();
	Direction.Normalize();

	const float Dot = FVector::DotProduct(OwnerActor->GetActorForwardVector(), Direction);
	const float AngleDeg = FMath::RadiansToDegrees(FMath::Acos(Dot));

	// 시야각 안에 들어왔는지 여부
	return 0.0f <= AngleDeg && AngleDeg <= InAngle;
}

void USightSystem::AddElapsedTime()
{
	ElapsedTime += GetWorld()->GetDeltaSeconds();
}

float USightSystem::LerpAlpha() const
{
	return UKismetMathLibrary::FClamp( ElapsedTime / Duration, 0.0f, 1.0f );
}
