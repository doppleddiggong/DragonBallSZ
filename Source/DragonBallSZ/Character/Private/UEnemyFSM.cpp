// Fill out your copyright notice in the Description page of Project Settings.


#include "UEnemyFSM.h"

#include <ThirdParty/ShaderConductor/ShaderConductor/External/DirectXShaderCompiler/include/dxc/DXIL/DxilConstants.h>

#include "AEnemyActor.h"
#include "APlayerActor.h"
#include "DragonBallSZ.h"
#include "VectorTypes.h"
#include "Features/UEaseFunctionLibrary.h"
#include "Shared/FEaseHelper.h"

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

	if ( !Itself->IsCombatStart() )
		return;
	
	CurrentTime += DeltaTime;
	
	if (bDamaged) // 피격 중에는 선택하지 않는다.
	{
		return;
	}
	else if (CurrentState == EEnemyState::Damaged) // 피격 시 행동 취소
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
	
	if (bActing) // 행동 수행 중에는 선택하지 않는다.
	{
		return;
	}
	
	if (CurrentTime > DecisionTime) // 시간이 됐으면 행동을 선택한다.
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
	return States[States.IndexOfByPredicate([&](const TPair<EEnemyState, float>& P)
	{
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
		MoveSpeed = 1000.f;
		if (IsValid(Itself) == false)
		{
			return;
		}

		CumulativeDistance += MoveSpeed * GetWorld()->GetDeltaSeconds();
		float t = FindT(CumulativeDistance);
		FVector Pos = Bezier(Origin, CenterControlPoint, Destination, t);

		// Approach distance tolerance
		if (ArcLength[Samples] < 10)
		{
			bMoving = false;
			bActing = true;
			return;
		}
		Itself->SetActorLocation(Pos, true);
		return;
	}

	// Moving
	MoveSpeed = 500.f;
	if (IsValid(Itself) == false)
	{
		return;
	}
	FVector DestinationDirection = Destination - Itself->GetActorLocation();
	DestinationDirection.Normalize();
	DestinationDirection.Z = 0;
	Itself->AddMovementInput(DestinationDirection, MoveSpeed);

	if (TargetDistance < 10)
	{
		bMoving = false;
	}
}

FVector UEnemyFSM::Bezier(const FVector Pa, const FVector ControlPoint, const FVector Pb, const float t)
{
	FVector Lerp1 = FMath::Lerp(Pa, ControlPoint, t);
	FVector Lerp2 = FMath::Lerp(ControlPoint, Pb, t);
	return FMath::Lerp(Lerp1, Lerp2, t);
}

void UEnemyFSM::BuildTable(FVector A, FVector P, FVector B)
{
	Samples = floor(TargetDistance / 10 + 0.5);
	ArcLength.SetNum(Samples + 1);
	ArcLength[0] = 0;
	FVector Prev = A;
	for (int i = 1; i <= Samples; i++)
	{
		float t = static_cast<float>(i) / Samples;
		FVector Cur = Bezier(A, P, B, t);
		ArcLength[i] = ArcLength[i - 1] + FVector::Dist(Prev, Cur);
		Prev = Cur;
	}
}

float UEnemyFSM::FindT(float s)
{
	for (int i = 1; i <= Samples; i++)
	{
		if (s <= ArcLength[i])
		{
			float ratio = (s - ArcLength[i - 1]) / (ArcLength[i] - ArcLength[i - 1]);
			return ((i - 1) + ratio) / Samples;
		}
	}
	return 1.0f;
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
	Origin = Itself->GetActorLocation();

	// Center Control Point Direction
	FVector SideVectorFromResult = FVector::CrossProduct(FVector::UpVector, Destination - Origin);
	FVector UpVectorFromResult = FVector::CrossProduct(Destination - Origin, SideVectorFromResult);
	UpVectorFromResult.Normalize();

	// CenterControlPoint = CenterPoint + Up or Down Vector
	float Height = Destination.Z - Origin.Z;
	float CurveHeight = FMath::RandRange(Height / -1, Height / 1);
	CenterControlPoint = (Destination + Origin) * 0.5f + UpVectorFromResult * CurveHeight;

	// Sample Curve Length
	TargetDistance = FVector::Dist(Destination, Origin);
	BuildTable(Origin, CenterControlPoint, Destination);

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
