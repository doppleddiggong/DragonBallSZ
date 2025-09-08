// Fill out your copyright notice in the Description page of Project Settings.


#include "UEnemyFSM.h"
#include "DragonBallSZ.h"

UEnemyFSM::UEnemyFSM()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UEnemyFSM::BeginPlay()
{
	Super::BeginPlay();
}

void UEnemyFSM::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	CurrentTime += DeltaTime;
	
	if (CurrentState == EEnemyState::Damaged)	// 피격 시 행동 취소
	{
		return;
	}
	if (bActing)	// 행동 수행 중에는 선택하지 않는다.
	{
		return;
	}
	if (CurrentTime > DecisionTime)	// 시간이 됐으면 행동을 선택한다.
	{
		CurrentTime = 0;
	}
	
	// States.Add({EEnemyState::Act, 100.f});	// 가중치 추가
	CurrentState = SelectWeightedRandomState();
	switch (CurrentState)
	{
	case EEnemyState::Idle:
		Idle();
		break;
	case EEnemyState::Move:
		Move();
		break;
	case EEnemyState::Damaged:
		Damaged();
		break;
	case EEnemyState::EnemyWin:
		PlayerDown();
		break;
	case EEnemyState::EnemyLose:
		EnemyDown();
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

void UEnemyFSM::Idle()
{
	PRINTINFO();
}



void UEnemyFSM::Damaged()
{
	PRINTINFO();
}

void UEnemyFSM::PlayerDown()
{
	PRINTINFO();
}

void UEnemyFSM::EnemyDown()
{
	PRINTINFO();
}

void UEnemyFSM::Move()
{
}

void UEnemyFSM::Attack()
{
	PRINTINFO();
}

void UEnemyFSM::Charge()
{
	PRINTINFO();
}

void UEnemyFSM::Skill()
{
	PRINTINFO();
}

