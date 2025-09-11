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

// MoveType
// (Sub State of ActDecision)
UENUM(BlueprintType)
enum class SpecialType : uint8
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
	// EEnemyState Weights
	EEnemyState SelectWeightedRandomState();
	TArray<TPair<EEnemyState, float>> States = {
		{EEnemyState::Idle, 10.f},
		{EEnemyState::Move, 150.f},
		{EEnemyState::Attack, 10.f},
		{EEnemyState::Charge, 10.f},
		{EEnemyState::Special, 0.f},
	};
	EEnemyState CurrentState = EEnemyState::Idle;
	bool bDamaged = false;
	bool bDefeated = false;
	bool bActing = false;
	float CurrentTime = 0;
	float DecisionTime = 1.5f;
	
	void Idle();
	void Move();
	FVector Bezier(FVector Pa, FVector ControlPoint, FVector Pb, float t);
	TArray<float> ArcLength;
	int Samples;
	void BuildTable(FVector A, FVector P, FVector B);
	float FindT(float s);
	float CumulativeDistance = 0;
	void TargetingDestination();
	float TargetDistance;
	FVector Origin;
	FVector Destination;
	FVector CenterControlPoint;
	UPROPERTY(EditAnywhere)
	bool bMoving;
	UPROPERTY(EditAnywhere)
	bool bFlying = false;
	UPROPERTY(EditAnywhere)
	float MoveSpeed;
	void Attack();
	void Charge();
	void Special();
	void Damaged();
	void EnemyWin();
	void EnemyLose();

	UPROPERTY(EditAnywhere, Category="TestTarget")
	TSubclassOf<AActor> TestFactory;
	UPROPERTY()
	TObjectPtr<AActor> TestTarget;
};
