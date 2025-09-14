// Fill out your copyright notice in the Description page of Project Settings.


#include "UEnemyFSM.h"

#include <ThirdParty/ShaderConductor/ShaderConductor/External/DirectXShaderCompiler/include/dxc/DXIL/DxilConstants.h>

#include "AEnemyActor.h"
#include "APlayerActor.h"
#include "DragonBallSZ.h"
#include "EnergyBlastActor.h"
#include "NiagaraFunctionLibrary.h"
#include "UDashSystem.h"
#include "UFlySystem.h"
#include "URushAttackSystem.h"
#include "VectorTypes.h"
#include "Features/UEaseFunctionLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
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

	if (Itself->IsCombatStart() == false ||
		Itself->IsCombatResult())
	{
		// 전투 시작전
		// 전투 결과후
		return;
	}

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
		if (TargetDistance > LongDistance)
		{
			//Itself->RushAttackSystem->bIsDashing = true;
			// 현재 상태 출력
			FString distStr = FString::Printf(TEXT("%hhd"), Itself->RushAttackSystem->bIsDashing);
			PRINTLOG(TEXT("%s"), *distStr);
			GEngine->AddOnScreenDebugMessage(0, 1, FColor::Cyan, distStr);
			BeizerMove();
			return;
		}

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
		case EMoveInputType::Jump:
			if (Itself->GetCharacterMovement()->MovementMode != MOVE_Flying) // Not flying: Jump & Fly
			{
				Itself->FlySystem->OnJump();
				FTimerHandle JumpTimer;
				GetWorld()->GetTimerManager().SetTimer(
					JumpTimer,
					[this]()
					{
						Itself->FlySystem->OnJump();
					},
					0.5f,
					false
				);
			}
			else // Land on ground
			{
				Itself->FlySystem->OnJump();
			};
			break;
		}
		return;
	}

	// Distance between Target
	TargetDistance = FVector::Dist(Itself->GetActorLocation(), Target->GetActorLocation());

	// 현재 상태 출력
	FString distStr = FString::Printf(TEXT("%f"), TargetDistance);
	PRINTLOG(TEXT("%s"), *distStr);
	GEngine->AddOnScreenDebugMessage(0, 1, FColor::Cyan, distStr);

	CurrentTime += DeltaTime;
	if (CurrentTime < DecisionTime) return; // 시간이 됐으면 행동을 선택한다.
	CurrentTime = 0;

	ModifyWeightArray(); // Add & Remove Weight

	ChangeState(SelectWeightedRandomState()); // Change state randomly

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

void UEnemyFSM::ModifyWeightArray()
{
	// Enemy Flying == Player Flying -> Jump: 0.f
	// Enemy Flying != Player Flying -> Jump: 200.f
	if (auto* JumpMove = Moves.FindByPredicate([](const auto& Elem)
	{
		return Elem.Key == EMoveInputType::Jump;
	}))
	{
		JumpMove->Value = ((Itself->GetCharacterMovement()->MovementMode == MOVE_Flying) != (Target->
			                  GetCharacterMovement()->MovementMode == MOVE_Flying))
			                  ? 300.f
			                  : 0.f;
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
	if (TargetDistance > LongDistance)
	{
		TargetingDestination();
		ActivateDashVFX(true);
		return;
	}

	if (FMath::FRand() <= FireRate)
	{
		SpawnEnergyBlastLoop(FMath::RandRange(1, 6));
	}

	// EMovementMode::MOVE_Flying
	CurrentMove = SelectWeightedRandomMove();

	switch (CurrentMove)
	{
	case EMoveInputType::Forward:
		MovingTime = FMath::RandRange(0.7f, 1.7f);
		break;
	case EMoveInputType::Backward:
		MovingTime = FMath::RandRange(0.3f, 1.5f);
		break;
	case EMoveInputType::Left:
		MovingTime = FMath::RandRange(0.9f, 2.5f);
		break;
	case EMoveInputType::Right:
		MovingTime = FMath::RandRange(0.9f, 2.5f);
		break;
	case EMoveInputType::Jump:
		MovingTime = 0;
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
		false,
		FMath::RandRange(0.f, 0.5f)
	);
}

void UEnemyFSM::ActivateDashVFX(bool Active)
{
	if (Active)
	{
		Itself->DashSystem->NiagaraComp->Activate(Active);
		return;
	}
	Itself->DashSystem->NiagaraComp->Deactivate();
}

void UEnemyFSM::CheckToLand()
{
	FHitResult Hit;
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		Itself->GetActorLocation(),
		Itself->GetActorLocation() - FVector(0, 0, 35.f),
		ECC_WorldDynamic
	);
	if (Hit.GetActor()->ActorHasTag("Ground")) Itself->FlySystem->OnLand(Hit);
}

FVector UEnemyFSM::Bezier(const FVector Pa, const FVector ControlPoint, const FVector Pb, const float t)
{
	FVector Lerp1 = FMath::Lerp(Pa, ControlPoint, t);
	FVector Lerp2 = FMath::Lerp(ControlPoint, Pb, t);
	return FMath::Lerp(Lerp1, Lerp2, t);
}

void UEnemyFSM::BeizerMove()
{
	// Fly
	if (Itself->GetCharacterMovement()->MovementMode != MOVE_Flying) Itself->GetCharacterMovement()->
	                                                                         SetMovementMode(MOVE_Flying);

	CumulativeDistance += MoveSpeed * GetWorld()->GetDeltaSeconds();
	float t = FindT(CumulativeDistance);
	FVector Pos = Bezier(OriginLocation, CenterControlPoint, Destination, t);

	// Approach distance tolerance
	if (ArcLength[Samples] < CumulativeDistance)
	{
		bMoving = false;
		CumulativeDistance = 0;
		ActivateDashVFX(false);
		CheckToLand();
		return;
	}
	Itself->SetActorLocation(Pos, true);
	Itself->DashSystem->NiagaraComp->SetWorldRotation(Itself->GetActorForwardVector().Rotation());
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
	OriginLocation = Itself->GetActorLocation();
	Destination = Target->GetActorLocation() + Target->GetActorForwardVector() * 1200;

	// Center Control Point Direction
	FVector SideVectorFromResult = FVector::CrossProduct(FVector::UpVector, Destination - OriginLocation);
	FVector UpVectorFromResult = FVector::CrossProduct(Destination - OriginLocation, SideVectorFromResult);
	UpVectorFromResult.Normalize();

	// CenterControlPoint = CenterPoint + Up or Down Vector
	float Height = Destination.Z - OriginLocation.Z;
	float CurveHeight = FMath::RandRange(0.f, Height / 2);
	CenterControlPoint = (Destination + OriginLocation) * 0.5f + UpVectorFromResult * CurveHeight;

	// Sample Curve Length
	DestinationDistance = FVector::Dist(Destination, OriginLocation);
	BuildTable(OriginLocation, CenterControlPoint, Destination);

	bMoving = true;
}
