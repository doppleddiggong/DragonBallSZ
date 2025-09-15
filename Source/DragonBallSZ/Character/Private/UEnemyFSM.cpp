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

	if (Itself->IsHold() )
		return;
	
	if (bDefeated)
		return; // Return if the Game is Over

	if (Itself->IsCombatStart() == false ||
		Itself->IsCombatResult())
	{
		// 전투 시작전
		// 전투 결과후
		return;
	}

	if (Itself->IsHit)
	{
		void Damaged();
		return;
	}

	if (Target->StatSystem->IsDead)
	{
		void EnemyWin();
		bDefeated = true;
		return;
	}
	else if (Itself->StatSystem->IsDead)
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
			// ToDo: LookAt 비활성화
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
			{
				// Preventing Z-Axis Overlap
				FVector TargetLocation = FVector(Target->GetActorLocation().X, Target->GetActorLocation().Y, 0.f);
				FVector ItselfLocation = FVector(Itself->GetActorLocation().X, Itself->GetActorLocation().Y, 0.f);
				float ProjectionDistance = FVector::Dist(TargetLocation, ItselfLocation);
				if (ProjectionDistance > 10) Itself->AddMovementInput(Itself->GetActorForwardVector(), 1);
				break;
			}
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
	// PRINTLOG(TEXT("Distance: %s"), *distStr);
	GEngine->AddOnScreenDebugMessage(0, 1, FColor::Cyan, distStr);

	CurrentTime += DeltaTime;
	if (CurrentTime < DecisionTime) return; // 시간이 됐으면 행동을 선택한다.
	CurrentTime = 0;

	ModifyWeightArray(); // Add & Remove Weight

	ChangeState(SelectWeightedRandomState()); // Change state randomly

	// 현재 상태 출력
	FString stateStr = UEnum::GetValueAsString(CurrentState);
	// PRINTLOG(TEXT("EnemyState %s"), *stateStr);
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
	// TargetDistance > MeleeDistance -> Attack: 2.f
	// TargetDistance < MeleeDistance -> Attack: 75.f
	if (auto* AttackState = States.FindByPredicate([](const auto& Elem)
	{
		return Elem.Key == EEnemyState::Attack;
	}))
	{
		AttackState->Value = (TargetDistance < MeleeDistance)
							  ? 75.f
							  : 2.f;
	}
	
	// Enemy Flying == Player Flying -> Jump: 0.f
	// Enemy Flying != Player Flying -> Jump: 150.f
	if (auto* JumpMove = Moves.FindByPredicate([](const auto& Elem)
	{
		return Elem.Key == EMoveInputType::Jump;
	}))
	{
		JumpMove->Value = ((Itself->GetCharacterMovement()->MovementMode == MOVE_Flying) != (Target->
			                  GetCharacterMovement()->MovementMode == MOVE_Flying))
			                  ? 150.f
			                  : 0.f;
	}

	// Enemy CurrentHP > Player CurrentHP -> Backward: 10.f
	// Enemy CurrentHP < Player CurrentHP -> Backward: 70.f
	if (auto* ApproachMove = Moves.FindByPredicate([](const auto& Elem)
	{
		return Elem.Key == EMoveInputType::Backward;
	}))
	{
		ApproachMove->Value = (Itself->StatSystem->CurHP > Target->StatSystem->CurHP)
							  ? 10.f
							  : 70.f;
	}

	// Enemy CurrentHP > Player CurrentHP -> Left, Right: 40.f
	// Enemy CurrentHP < Player CurrentHP -> Left, Right: 70.f
	for (auto& Elem : Moves)
	{
		if (Elem.Key == EMoveInputType::Left || Elem.Key == EMoveInputType::Right)
		{
			Elem.Value = (Itself->StatSystem->CurHP > Target->StatSystem->CurHP)
						 ? 40.f
						 : 70.f;
		}
	}

	// Enemy CurrentHP < Player CurrentHP -> FireRate: 0.2 -> 0.4
	Itself->StatSystem->CurHP > Target->StatSystem->CurHP ? FireRate = 0.2f : FireRate = 0.4f;
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
		MovingTime = FMath::RandRange(0.5f, 1.3f);
		break;
	case EMoveInputType::Backward:
		MovingTime = FMath::RandRange(0.3f, 1.1f);
		break;
	case EMoveInputType::Left:
		MovingTime = FMath::RandRange(0.7f, 1.6f);
		break;
	case EMoveInputType::Right:
		MovingTime = FMath::RandRange(0.7f, 1.6f);
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
	Itself->RushAttackSystem->OnAttack();
	
	// Follow Player flying state
	if ((Itself->GetCharacterMovement()->MovementMode == MOVE_Flying) != (Target->GetCharacterMovement()->MovementMode == MOVE_Flying)) Itself->GetCharacterMovement()->SetMovementMode((Target->GetCharacterMovement()->MovementMode == MOVE_Flying) ? MOVE_Flying : MOVE_Walking);
	
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
