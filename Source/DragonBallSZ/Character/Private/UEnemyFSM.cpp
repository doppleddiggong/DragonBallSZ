// Fill out your copyright notice in the Description page of Project Settings.


#include "UEnemyFSM.h"

#include <ThirdParty/ShaderConductor/ShaderConductor/External/DirectXShaderCompiler/include/dxc/DXIL/DxilConstants.h>

#include "AEnemyActor.h"
#include "APlayerActor.h"
#include "DragonBallSZ.h"
#include "EnergyBlastActor.h"
#include "VectorTypes.h"
#include "Features/UEaseFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Runtime/MovieSceneTracks/Private/MovieSceneTracksCustomAccessors.h"
#include "Shared/FEaseHelper.h"

UEnemyFSM::UEnemyFSM()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UEnemyFSM::BeginPlay()
{
	Super::BeginPlay();

	Itself = Cast<AEnemyActor>(GetOwner());

	Target = Cast<APlayerActor>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	ensureMsgf(Target, TEXT("UEnemyFSM: Target 캐스팅 실패! APlayerActor가 필요합니다!"));
}

void UEnemyFSM::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bDefeated) return; // Return if the Game is Over

	if (CurrentState == EEnemyState::Damaged)
	{
		void Damaged();
		return;
	}

	if (CurrentState == EEnemyState::EnemyWin)
	{
		void EnemyWin();
		bDefeated = true;
		return;
	}
	else if (CurrentState == EEnemyState::EnemyLose)
	{
		void EnemyLose();
		bDefeated = true;
		return;
	}

	if (bActing) return; // 행동 수행 중에는 선택하지 않는다.

	if (bMoving)
	{
		ElapsedMoving += DeltaTime;
		// if (TargetDistance > LongDistance)
		// {
		// 	MoveBeizer();
		// 	return;
		// }

		if (ElapsedMoving > MovingTime)
		{
			bMoving = false;
			ElapsedMoving = 0;
		}

		switch (CurrentMove)
		{
		case EMoveInputType::Forward:
			Itself->AddMovementInput(Itself->GetActorForwardVector(), 1);
			break;
		case EMoveInputType::Backward:
			Itself->AddMovementInput(Itself->GetActorForwardVector(), -1);
			break;
		case EMoveInputType::Left:
			Itself->AddMovementInput(Itself->GetActorRightVector(), -1);
			break;
		case EMoveInputType::Right:
			Itself->AddMovementInput(Itself->GetActorRightVector(), 1);
			break;
		}
		return;
	}

	// Distance between Target
	TargetDistance = FVector::Dist(Itself->GetActorLocation(), Target->GetActorLocation());

	CurrentTime += DeltaTime;
	if (CurrentTime < DecisionTime) return; // 시간이 됐으면 행동을 선택한다.
	CurrentTime = 0;


	// States.Add({EEnemyState::Act, 100.f});	// 가중치 추가
	ChangeState(SelectWeightedRandomState());

	// 현재 상태 출력
	FString stateStr = UEnum::GetValueAsString(CurrentState);
	PRINTLOG(TEXT("%s"), *stateStr);
	GEngine->AddOnScreenDebugMessage(0, 1, FColor::Cyan, stateStr);

	switch (CurrentState)
	{
	case EEnemyState::Idle:
		Idle();
		break;
	case EEnemyState::Move:
		//TargetingDestination();	// Beizer 이동
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
	float TotalWeight = 0.f;
	for (auto& Pair : States) TotalWeight += Pair.Value;

	float RandomValue = FMath::FRandRange(0.f, TotalWeight);

	float Accumulation = 0.f;
	return States[States.IndexOfByPredicate([&](const TPair<EEnemyState, float>& P)
	{
		Accumulation += P.Value;
		return RandomValue <= Accumulation;
	})].Key;
}

EMoveInputType UEnemyFSM::SelectWeightedRandomMove()
{
	float TotalWeight = 0.f;
	for (auto& Pair : Moves) TotalWeight += Pair.Value;

	float RandomValue = FMath::FRandRange(0.f, TotalWeight);

	float Accumulation = 0.f;
	return Moves[Moves.IndexOfByPredicate([&](const TPair<EMoveInputType, float>& P)
	{
		Accumulation += P.Value;
		return RandomValue <= Accumulation;
	})].Key;
}

void UEnemyFSM::ChangeState(EEnemyState NewState)
{
	CurrentState = NewState;
}

// Act
void UEnemyFSM::Idle()
{
	PRINTINFO();
}

void UEnemyFSM::Move()
{
	// if (TargetDistance > LongDistance) TargetingDestination();

	CurrentMove = SelectWeightedRandomMove();

	if (FMath::FRand() <= FireRate)
	{
		SpawnEnergyBlastLoop(FMath::RandRange(1, 6));
	}
	

	switch (CurrentMove)
	{
	case EMoveInputType::Forward:
		MovingTime = FMath::RandRange(0.7f, 2.3f);
		break;
	case EMoveInputType::Backward:
		MovingTime = FMath::RandRange(1.1f, 4.3f);
		break;
	case EMoveInputType::Left:
		MovingTime = FMath::RandRange(1.1f, 4.3f);
		break;
	case EMoveInputType::Right:
		MovingTime = FMath::RandRange(0.5f, 1.9f);
		break;
	}

	bMoving = true;
}


void UEnemyFSM::Attack()
{
	bActing = true;


	PRINTINFO();
	bActing = false;
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

void UEnemyFSM::SpawnEnergyBlast()
{
	FActorSpawnParameters Params;
	Params.Owner = Itself;
	Params.Instigator = Itself;

	GetWorld()->SpawnActor<AEnergyBlastActor>(
		Itself->EnergyBlastFactory,
		Itself->GetActorTransform(),
		Params
	);
}

void UEnemyFSM::SpawnEnergyBlastLoop(int32 Remaining)
{
	if (Remaining <= 0) return;

	SpawnEnergyBlast();
	
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindLambda([this, Remaining]()
	{
		SpawnEnergyBlastLoop(Remaining - 1);
	});

	GetWorld()->GetTimerManager().SetTimer(
		EnergyBlastTimer,
		TimerDelegate,
		0.17f,
		false
	);
}

void UEnemyFSM::MoveBeizer()
{
	// Flying
	MoveSpeed = 10000.f;
	if (IsValid(Itself) == false)
	{
		return;
	}

	CumulativeDistance += MoveSpeed * GetWorld()->GetDeltaSeconds();
	float t = FindT(CumulativeDistance);
	FVector Pos = Bezier(Origin, CenterControlPoint, Destination, t);

	// 현재 상태 출력
	FString DistStr = FString::Printf(TEXT("%.2f"), CumulativeDistance);
	PRINTLOG(TEXT("%s"), *DistStr);
	GEngine->AddOnScreenDebugMessage(1, 1, FColor::Cyan, DistStr);

	// Approach distance tolerance
	if (ArcLength[Samples] < 10)
	{
		CumulativeDistance = 0;
		bMoving = false;
		bActing = true;
		return;
	}
	Itself->SetActorLocation(Pos, true);
}

FVector UEnemyFSM::Bezier(const FVector Pa, const FVector ControlPoint, const FVector Pb, const float t)
{
	FVector Lerp1 = FMath::Lerp(Pa, ControlPoint, t);
	FVector Lerp2 = FMath::Lerp(ControlPoint, Pb, t);
	return FMath::Lerp(Lerp1, Lerp2, t);
}

void UEnemyFSM::BuildTable(FVector A, FVector P, FVector B)
{
	Samples = floor(DestinationDistance / 10 + 0.5);
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
	Origin = Itself->GetActorLocation();
	Destination = Target->GetActorLocation() + (Origin - Target->GetActorLocation()).GetSafeNormal() * 2000;

	// Center Control Point Direction
	FVector SideVectorFromResult = FVector::CrossProduct(FVector::UpVector, Destination - Origin);
	FVector UpVectorFromResult = FVector::CrossProduct(Destination - Origin, SideVectorFromResult);
	UpVectorFromResult.Normalize();

	// CenterControlPoint = CenterPoint + Up or Down Vector
	float Height = Destination.Z - Origin.Z;
	float CurveHeight = FMath::RandRange(0.f, Height / 2);
	CenterControlPoint = (Destination + Origin) * 0.5f + UpVectorFromResult * CurveHeight;

	// Sample Curve Length
	DestinationDistance = FVector::Dist(Destination, Origin);
	BuildTable(Origin, CenterControlPoint, Destination);

	bMoving = true;
}
