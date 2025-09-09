// Fill out your copyright notice in the Description page of Project Settings.


#include "UEnemyFSM.h"

#include <ThirdParty/ShaderConductor/ShaderConductor/External/DirectXShaderCompiler/include/dxc/DXIL/DxilConstants.h>

#include "AEnemyActor.h"
#include "APlayerActor.h"
#include "DragonBallSZ.h"

UEnemyFSM::UEnemyFSM()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UEnemyFSM::BeginPlay()
{
	Super::BeginPlay();

	//Target = Cast<APlayerActor>(GetWorld()->GetFirstPlayerController());

	Itself = Cast<AEnemyActor>(GetOwner());

	TestTarget = GetWorld()->SpawnActor<AActor>(TestFactory, FVector::ZeroVector, FRotator::ZeroRotator);
}

void UEnemyFSM::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	CurrentTime += DeltaTime;

	if (bDamaged)	// 피격 중에는 선택하지 않는다.
	{
		return;
	}
	else if (CurrentState == EEnemyState::Damaged)	// 피격 시 행동 취소
	{
		void Damaged();
		return;
	}

	if (bDefeated)
	{
		return;
	}
	else if (CurrentState == EEnemyState::EnemyWin)
	{
		void EnemyWin();
		return;
	}
	else if (CurrentState == EEnemyState::EnemyLose)
	{
		void EnemyLose();
		return;
	}
	
	if (bActing)	// 행동 수행 중에는 선택하지 않는다.
	{
		return;
	}
	
	if (CurrentTime > DecisionTime)	// 시간이 됐으면 행동을 선택한다.
	{
		return;
	}
	CurrentTime = 0;
	
	// States.Add({EEnemyState::Act, 100.f});	// 가중치 추가
	if (!bMoving)
	{
		CurrentState = SelectWeightedRandomState();
	}
	
	switch (CurrentState)
	{
	case EEnemyState::Idle:
		Idle();
		break;
	case EEnemyState::Move:
		TargetingDestination();
		Move();
		break;
	case EEnemyState::Attack:
		Attack();
		break;
	case EEnemyState::Charge:
		Charge();
		break;
	case EEnemyState::Special:
		Special();
		break;
	}
}

EEnemyState UEnemyFSM::SelectWeightedRandomState()
{
	// 총합 계산
	float TotalWeight = 0.f;
	for (auto& Pair : States) TotalWeight += Pair.Value;

	// 랜덤값 0~총합
	float RandomValue = FMath::FRandRange(0.f, TotalWeight);
	
	// 한 줄 누적합 비교로 선택
	float Accumulation = 0.f;
	return States[States.IndexOfByPredicate([&](const TPair<EEnemyState, float>& P){
		Accumulation += P.Value;
		return RandomValue <= Accumulation;
	})].Key;
}

// Act
void UEnemyFSM::Idle()
{
	PRINTINFO();
}

void UEnemyFSM::Move()
{
	// Flying
	if (bFlying)
	{
		MoveSpeed = 1.f;
		if (IsValid(Itself) == false)
		{
			return;
		}
		float Distance = FVector::Dist(Destination, Itself->GetActorLocation());
		AlphaProgress += (GetWorld()->GetDeltaSeconds() * MoveSpeed) / Distance;
		AlphaProgress = FMath::Clamp(AlphaProgress, 0.0f, 1.0f);
		
		float alpha = GetWorld()->GetDeltaSeconds()* MoveSpeed;	//  얼마나 빨리
		FVector Pa = FMath::Lerp(Itself->GetActorLocation(), CenterControlPoint, AlphaProgress);
		FVector Pb = FMath::Lerp(CenterControlPoint, Destination, AlphaProgress);
		FVector P = FMath::Lerp(Pa, Pb, alpha);
		
		// 원충돌
		
		if (AlphaProgress >= 1.0f)
		{
			AlphaProgress = 0;
			bMoving = false;
			bActing = true;
			return;
		}
		Itself->SetActorLocation(P, true);
		return;
	}
	
	// Moving
	MoveSpeed =500.f;
	if (IsValid(Itself) == false)
	{
		return;
	}
	FVector DestinationDirection = Destination - Itself->GetActorLocation();
	DestinationDirection.Normalize();
	DestinationDirection.Z = 0;
	Itself->AddMovementInput(DestinationDirection, MoveSpeed);
	
	float dist = FVector::Dist(DestinationDirection, Itself->GetActorLocation());
	if (dist < 10)
	{
		bMoving = false;
	}
}

void UEnemyFSM::TargetingDestination()
{
	if (bMoving)
	{
		return;
	}
	
	// 랜덤한 위치에서 가져오고 싶다.
	// if (TestTarget)
	// {
	// 	Destination = TestTarget->GetActorLocation();
	// }
	Destination = FVector(0, 0, 1000);

	// Center Control Point Direction
	FVector SideVectorFromResult = FVector::CrossProduct(FVector::UpVector, Destination - Itself->GetActorLocation());
	FVector UpVectorFromResult = FVector::CrossProduct(Destination - Itself->GetActorLocation(), SideVectorFromResult);
	UpVectorFromResult.Normalize();

	// CenterControlPoint = CenterPoint + Up or Down Vector
	float Height = Destination.Z - Itself->GetActorLocation().Z;
	float CurveHeight = FMath::RandRange(Height / -2, Height / 2);
	CenterControlPoint = (Destination + Itself->GetActorLocation()) * 0.5f + UpVectorFromResult * CurveHeight;

	UE_LOG(LogTemp, Warning, TEXT("%s"), *CenterControlPoint.ToString())
	bMoving = true;
}

void UEnemyFSM::Attack()
{
	bActing = true;
	PRINTINFO();
}

void UEnemyFSM::Charge()
{
	bActing = true;
	PRINTINFO();
}

void UEnemyFSM::Special()
{
	bActing = true;
	PRINTINFO();
}

// Damaged
void UEnemyFSM::Damaged()
{
	bDamaged = true;
	PRINTINFO();
}

// Defeated
void UEnemyFSM::EnemyWin()
{
	bDefeated = true;
	PRINTINFO();
}

void UEnemyFSM::EnemyLose()
{
	bDefeated = true;
	PRINTINFO();
}







