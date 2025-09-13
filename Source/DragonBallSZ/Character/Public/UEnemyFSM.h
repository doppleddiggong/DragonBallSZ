// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UEnemyFSM.generated.h"


class AEnemyActor;
class APlayerActor;

// Enemy State
UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	Idle UMETA(DisplayName = "IdleState"),
	Move UMETA(DisplayName = "MoveState"),
	Attack UMETA(DisplayName = "MeleeAttackState"),
	Charge UMETA(DisplayName = "ChargeKiState"),
	Special UMETA(DisplayName = "SpecialAttackState"),
	Damaged UMETA(DisplayName = "DamagedState"),
	EnemyWin UMETA(DisplayName = "EnemyWinState"),
	EnemyLose UMETA(DisplayName = "EnemyLoseState"),
};

UENUM(BlueprintType)
enum class EMoveInputType : uint8
{
	Forward UMETA(DisplayName = "MoveForward"),
	Backward UMETA(DisplayName = "MoveBackward"),
	Left UMETA(DisplayName = "MoveLeft"),
	Right UMETA(DisplayName = "MoveRight"),
};


// MoveType
// (Sub State of ActDecision)
UENUM(BlueprintType)
enum class ESpecialType : uint8
{
	Kamehameha UMETA(DisplayName = "Kamehameha"),
	ReleaseKi UMETA(DisplayName = "ReleaseKnockbackKi"),
};


UCLASS(Blueprintable, ClassGroup=(DBSZ), meta=(BlueprintSpawnableComponent))
class DRAGONBALLSZ_API UEnemyFSM : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnemyFSM();
	
	UPROPERTY()
	TObjectPtr<APlayerActor> Target;
	UPROPERTY()
	TObjectPtr<AEnemyActor> Itself;

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

public:
	EEnemyState SelectWeightedRandomState();	// EEnemyState Weights
	TArray<TPair<EEnemyState, float>> States = {
		{EEnemyState::Idle, 10.f},
		{EEnemyState::Move, 10.f},
		{EEnemyState::Attack, 70.f},
		{EEnemyState::Charge, 0.f},
		{EEnemyState::Special, 0.f},
	};
	EEnemyState CurrentState = EEnemyState::Idle;
	void ChangeState(EEnemyState NewState);
	bool bDamaged = false;
	bool bDefeated = false;
	bool bActing = false;
	float CurrentTime = 0;
	float ElapsedMoving = 0;
	float DecisionTime = 1.f;
	float MovingTime;
	float TargetDistance;
	UPROPERTY(EditAnywhere)
	float LongDistance = 2000;
	
	void Idle();
	
	void Move();
	EMoveInputType SelectWeightedRandomMove();	// EEnemyState Weights
	TArray<TPair<EMoveInputType, float>> Moves = {
		{EMoveInputType::Forward, 150.f},
		{EMoveInputType::Backward, 20.f},
		{EMoveInputType::Left, 90.f},
		{EMoveInputType::Right, 90.f},
	};
	EMoveInputType CurrentMove;
	
	void Attack();
	
	void Charge();
	
	void Special();
	
	void Damaged();
	
	void EnemyWin();
	
	void EnemyLose();

	void SpawnEnergyBlast();
	
	void MoveBeizer();	// Beizer Move
	FVector Bezier(FVector Pa, FVector ControlPoint, FVector Pb, float t);
	TArray<float> ArcLength;
	int Samples;
	void BuildTable(FVector A, FVector P, FVector B);
	float FindT(float s);
	float CumulativeDistance = 0;
	void TargetingDestination();
	float DestinationDistance;
	FVector Origin;
	FVector Destination;
	FVector CenterControlPoint;
	UPROPERTY(EditAnywhere)
	bool bMoving;
	UPROPERTY(EditAnywhere)
	bool bFlying = false;
	UPROPERTY(EditAnywhere)
	float MoveSpeed;
};
