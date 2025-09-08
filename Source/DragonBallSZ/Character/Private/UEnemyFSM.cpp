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
	case EEnemyState::Act:
		Act();
		break;
	case EEnemyState::Damaged:
		Damaged();
		break;
	case EEnemyState::PlayerDown:
		PlayerDown();
		break;
	case EEnemyState::EnemyDown:
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

void UEnemyFSM::Act()
{
	switch (CurrentAct)
	{
	case EActDecision::Move:
		Move();
		break;
	case EActDecision::Attack:
		Attack();
		break;
	case EActDecision::Charge:
		Charge();
		break;
	case EActDecision::Skill:
		Skill();
		break;
	}
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
	switch (CurrentMove)
	{
	case EMoveType::Approach:
		Approach();
		break;
	case EMoveType::SideStep:
		SideStep();
		break;
	case EMoveType::Escape:
		Escape();
		break;
	}
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

void UEnemyFSM::Approach()
{
	switch (CurrentApproach)
	{
	case EApproachType::NormalApproach:
		NormalApproach();
		break;
	case EApproachType::JogitanApproach:
		JogitanApproach();
		break;
	}
}
void UEnemyFSM::SideStep()
{
	switch (CurrentSideStep)
	{
	case ESideStepType::LeftStep:
		LeftStep();
		break;
	case ESideStepType::JogitanLeftStep:
		JogitanLeftStep();
		break;
	case ESideStepType::LeftDodge:
		LeftDodge();
		break;
	case ESideStepType::RightStep:
		RightStep();
		break;
	case ESideStepType::JogitanRightStep:
		JogitanRightStep();
		break;
	case ESideStepType::RightDodge:
		RightDodge();
		break;
	}
}
void UEnemyFSM::Escape()
{
	PRINTINFO();
}

void UEnemyFSM::NormalApproach()
{
	PRINTINFO();
}

void UEnemyFSM::JogitanApproach()
{
	PRINTINFO();
}

void UEnemyFSM::LeftStep()
{
	PRINTINFO();
}

void UEnemyFSM::JogitanLeftStep()
{
	PRINTINFO();
}

void UEnemyFSM::LeftDodge()
{
	PRINTINFO();
}

void UEnemyFSM::RightStep()
{
	PRINTINFO();
}

void UEnemyFSM::JogitanRightStep()
{
	PRINTINFO();
}

void UEnemyFSM::RightDodge()
{
	PRINTINFO();
}