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
};

UENUM(BlueprintType)
enum class EMoveInputType : uint8
{
	Forward UMETA(DisplayName = "MoveForward"),
	Backward UMETA(DisplayName = "MoveBackward"),
	Left UMETA(DisplayName = "MoveLeft"),
	Right UMETA(DisplayName = "MoveRight"),
	Jump UMETA(DisplayName = "Jump&Fly"),
};


// MoveType
// (Sub State of ActDecision)
UENUM(BlueprintType)
enum class ESpecialType : uint8
{
	Kamehameha UMETA(DisplayName = "Kamehameha"),
	KnockbackKi UMETA(DisplayName = "KnockbackKi"),
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
	TObjectPtr<AEnemyActor> Owner;

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

public:
	// States.Add({EEnemyState::Move, 100.f});	// 가중치 추가
	void ModifyWeightArray();
	EEnemyState SelectWeightedRandomState(); // EEnemyState Weights
	TArray<TPair<EEnemyState, float>> States = {
		{EEnemyState::Idle, 1.f},
		{EEnemyState::Move, 10.f},
		{EEnemyState::Attack, 0.f},
		{EEnemyState::Charge, 2.f},
		{EEnemyState::Special, 0.f},
	};
	EEnemyState CurrentState = EEnemyState::Idle;
	void ChangeState(EEnemyState NewState);
	bool bDamaged = false;
	bool bDefeated = false;
	bool bActing = false;
	float CurrentTime = 0;
	float ElapsedMoving = 0;
	float DecisionTime = 0.8f;    
	float MovingTime;
	float TargetDistance;
	float MeleeDistance = 1000;
	float LongDistance = 4500;

	void Idle();

	void Move();
	EMoveInputType SelectWeightedRandomMove(); // EEnemyState Weights
	TArray<TPair<EMoveInputType, float>> Moves = {
		{EMoveInputType::Forward, 80.f},
		{EMoveInputType::Backward, 10.f},
		{EMoveInputType::Left, 40.f},
		{EMoveInputType::Right, 40.f},
		{EMoveInputType::Jump, 0.f},
	};
	EMoveInputType CurrentMove;

	void Attack();

	void Charge();

	void Special();

	void Damaged();

	void EnemyWin();

	void EnemyLose();

	void SpawnEnergyBlast();
	void SpawnEnergyBlastLoop(int32 Remaining);
	FTimerHandle EnergyBlastTimer;
	UPROPERTY(EditAnywhere)
	float FireRate = 0.2f;

	void ActivateDashVFX(bool Active);
	void CheckToLand();
	FVector Bezier(FVector Pa, FVector ControlPoint, FVector Pb, float t);
	void BeizerMove();
	TArray<float> ArcLength;
	int Samples;
	void BuildTable(FVector A, FVector P, FVector B);
	float FindT(float s);
	float CumulativeDistance = 0;
	void TargetingDestination();
	float DestinationDistance;
	FVector Destination;
	FVector OriginLocation;
	FVector CenterControlPoint;
	bool bMoving;
	float MoveSpeed = 1700.f;
};
