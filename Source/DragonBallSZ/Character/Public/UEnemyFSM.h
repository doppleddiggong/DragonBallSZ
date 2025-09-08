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
	Move UMETA(DisplayName = "MoveState"),
	Attack UMETA(DisplayName = "AttackState"),
	Charge UMETA(DisplayName = "ChargeState"),
	Special UMETA(DisplayName = "SpecialAttackState"),
	Damaged UMETA(DisplayName = "DamagedState"),
	EnemyWin UMETA(DisplayName = "EnemyWinState"),
	EnemyLose UMETA(DisplayName = "EnemyLoseState"),
};

// MoveType
// (Sub State of ActDecision)
UENUM(BlueprintType)
enum class AttackType : uint8
{
	Approach UMETA(DisplayName = "ApproachMove"),
	SideStep UMETA(DisplayName = "SideStepMove"),

};

// MoveType
// (Sub State of ActDecision)
UENUM(BlueprintType)
enum class SpecialType : uint8
{
	Approach UMETA(DisplayName = "ApproachMove"),
	SideStep UMETA(DisplayName = "SideStepMove"),
	Escape UMETA(DisplayName = "EscapeMove"),
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
		{EEnemyState::Move, 200.f}
	};
	EEnemyState CurrentState = EEnemyState::Idle;
	bool bActing = false;
	float CurrentTime = 0;
	float DecisionTime = 1.5f;
	void Idle();
	void Damaged();
	void PlayerDown();
	void EnemyDown();
	void Move();
	void Attack();
	void Charge();
	void Skill();
	
};
