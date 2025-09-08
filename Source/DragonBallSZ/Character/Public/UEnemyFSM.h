// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UEnemyFSM.generated.h"

// Enemy State
UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	Idle UMETA(DisplayName = "IdleState"),
	Act UMETA(DisplayName = "ActState"),
	Damaged UMETA(DisplayName = "DamagedState"),
	PlayerDown UMETA(DisplayName = "PlayerDownState"),
	EnemyDown UMETA(DisplayName = "EnemyDownState"),
};

// ActDecision, Sub State of ActState
UENUM(BlueprintType)
enum class EActDecision : uint8
{
	Move UMETA(DisplayName = "MoveAct"),
	Attack UMETA(DisplayName = "AttackAct"),
	Charge UMETA(DisplayName = "ChargeAct"),
	Skill UMETA(DisplayName = "SkillAct"),
};

// MoveType
// (Sub State of ActDecision)
UENUM(BlueprintType)
enum class EMoveType : uint8
{
	Approach UMETA(DisplayName = "ApproachMove"),
	SideStep UMETA(DisplayName = "SideStepMove"),
	Escape UMETA(DisplayName = "EscapeMove"),
};

// ApproachType
UENUM(BlueprintType)
enum class EApproachType : uint8
{
	NormalApproach UMETA(DisplayName = "NormalApproach"),
	JogitanApproach UMETA(DisplayName = "JogitanApproach"),
};

// SideStepType
UENUM(BlueprintType)
enum class ESideStepType : uint8
{
	LeftStep UMETA(DisplayName = "LeftSideStep"),
	JogitanLeftStep UMETA(DisplayName = "JogitanLeftSideStep"),
	LeftDodge UMETA(DisplayName = "LeftDodge"),
	RightStep UMETA(DisplayName = "RightSideStep"),
	JogitanRightStep UMETA(DisplayName = "JogitanRightSideStep"),
	RightDodge UMETA(DisplayName = "RightDodge"),
};


UCLASS( Blueprintable, ClassGroup=(DBSZ), meta=(BlueprintSpawnableComponent) )
class DRAGONBALLSZ_API UEnemyFSM : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnemyFSM();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// EEnemyState Weights
	EEnemyState SelectWeightedRandomState();
	TArray<TPair<EEnemyState, float>> States = {
		{EEnemyState::Idle, 10.f},
		{EEnemyState::Act, 200.f}
	};
	EEnemyState CurrentState = EEnemyState::Idle;
	bool bActing = false;
	float CurrentTime = 0;
	float DecisionTime = 1.5f;
	void Idle();
	void Act();
	void Damaged();
	void PlayerDown();
	void EnemyDown();

	EActDecision CurrentAct = EActDecision::Move;
	void Move();
	void Attack();
	void Charge();
	void Skill();

	EMoveType CurrentMove = EMoveType::Approach;
	void Approach();
	void SideStep();
	void Escape();

	EApproachType CurrentApproach = EApproachType::NormalApproach;
	void NormalApproach();
	void JogitanApproach();

	ESideStepType CurrentSideStep = ESideStepType::LeftStep;
	void LeftStep();
	void JogitanLeftStep();
	void LeftDodge();
	void RightStep();
	void JogitanRightStep();
	void RightDodge();
};
