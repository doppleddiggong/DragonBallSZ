// Fill out your copyright notice in the Description page of Project Settings.


#include "UEnemyFSM.h"
#include "AEnemyActor.h"
#include "APlayerActor.h"

#include "UChargeKiSystem.h"
#include "UDashSystem.h"
#include "UFlySystem.h"
#include "URushAttackSystem.h"
#include "DragonBallSZ.h"

#include "VectorTypes.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"


UEnemyFSM::UEnemyFSM()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UEnemyFSM::BeginPlay()
{
	Super::BeginPlay();

	Owner = Cast<AEnemyActor>(GetOwner());

	Target = Cast<APlayerActor>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	ensureMsgf(Target, TEXT("UEnemyFSM: Target 캐스팅 실패! APlayerActor가 필요합니다!"));

	AnimInstance = Owner->GetMesh()->GetAnimInstance();
	if (Owner->GetCharacterType() == ECharacterType::Songoku)
	{
		DashMontage = GokuDashMontage;
	}
	else if (Owner->GetCharacterType() == ECharacterType::Vegeta)
	{
		DashMontage = VegetaDashMontage;
	}
}

void UEnemyFSM::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Owner->IsHolding())
		return;

	if (bDefeated)
		return; // Return if the Game is Over

	if (Owner->IsCombatStart() == false ||
		Owner->IsCombatResult())
	{
		// 전투 시작전
		// 전투 결과후
		return;
	}

	if (Owner->IsHitting())
	{
		void Damaged();
		return;
	}

	if (Target->IsDead())
	{
		void EnemyWin();
		bDefeated = true;
		return;
	}
	else if (Owner->IsDead())
	{
		void EnemyLose();
		bDefeated = true;
		return;
	}

	if (Owner->RushAttackSystem->bIsAttacking || Owner->ChargeKiSystem->IsActivateState()) return;
	// 행동 수행 중에는 선택하지 않는다.

	if (bMoving)
	{
		ElapsedMoving += DeltaTime;
		if (TargetDistance > LongDistance)
		{
			// ToDo: LookAt 비활성화
			
			BeizerMove();

			FVector Direction = Target->GetActorLocation() - Owner->GetActorLocation();
			FRotator BaseRot = Direction.Rotation();
			FRotator OffsetRot(90.f, 0.f, 0.f);
			if (Owner->DashSystem->DashWindComp) Owner->DashSystem->DashWindComp->SetWorldRotation(BaseRot + OffsetRot);

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
				FVector ItselfLocation = FVector(Owner->GetActorLocation().X, Owner->GetActorLocation().Y, 0.f);
				float ProjectionDistance = FVector::Dist(TargetLocation, ItselfLocation);

				if (ProjectionDistance > 90) Owner->AddMovementInput(Owner->GetActorForwardVector(), 1);
				break;
			}
		case EMoveInputType::Backward:
			Owner->AddMovementInput(Owner->GetActorForwardVector(), -1);
			break;
		case EMoveInputType::Left:
			Owner->AddMovementInput(Owner->GetActorRightVector(), -1);
			break;
		case EMoveInputType::Right:
			Owner->AddMovementInput(Owner->GetActorRightVector(), 1);
			break;
		case EMoveInputType::Jump:
			if (Owner->GetCharacterMovement()->MovementMode != MOVE_Flying) // Not flying: Jump & Fly
			{
				Owner->FlySystem->OnJump();

				FTimerHandle JumpTimer;
				GetWorld()->GetTimerManager().SetTimer(
					JumpTimer,
					[this]()
					{
						Owner->FlySystem->OnJump();
					},
					0.5f,
					false
				);
			}
			else // Land on ground
			{
				Owner->FlySystem->OnJump();
			};
			break;
		}
		return;
	}

	// Distance between Target
	TargetDistance = FVector::Dist(Owner->GetActorLocation(), Target->GetActorLocation());

	FString distStr = FString::SanitizeFloat(TargetDistance);
	GEngine->AddOnScreenDebugMessage(0, 1, FColor::Cyan, distStr);
	FString stateStr = UEnum::GetValueAsString(CurrentState);
	GEngine->AddOnScreenDebugMessage(1, 1, FColor::Cyan, stateStr);

	CurrentTime += DeltaTime;
	if (CurrentTime < DecisionTime) return; // 시간이 됐으면 행동을 선택한다.
	CurrentTime = 0;

	ModifyWeightArray(); // Add & Remove Weight

	ChangeState(SelectWeightedRandomState()); // Change state randomly

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
			                     : 0.7f;
	}

	// Enemy Flying == Player Flying -> Jump: 0.f
	// Enemy Flying != Player Flying -> Jump: 150.f
	if (auto* JumpMove = Moves.FindByPredicate([](const auto& Elem)
	{
		return Elem.Key == EMoveInputType::Jump;
	}))
	{
		JumpMove->Value = ((Owner->GetCharacterMovement()->MovementMode == MOVE_Flying) != (Target->
			                  GetCharacterMovement()->MovementMode == MOVE_Flying))
			                  ? 150.f
			                  : 0.f;
	}

	// Enemy CurrentHP > Player CurrentHP -> Backward: 10.f
	// Enemy CurrentHP < Player CurrentHP -> Backward: 60.f
	if (auto* ApproachMove = Moves.FindByPredicate([](const auto& Elem)
	{
		return Elem.Key == EMoveInputType::Backward;
	}))
	{
		ApproachMove->Value = (Owner->CurHP() > Target->CurHP())
			                      ? 10.f
			                      : 60.f;
	}

	// Enemy CurrentHP > Player CurrentHP -> Left, Right: 40.f
	// Enemy CurrentHP < Player CurrentHP -> Left, Right: 70.f
	for (auto& Elem : Moves)
	{
		if (Elem.Key == EMoveInputType::Left || Elem.Key == EMoveInputType::Right)
		{
			Elem.Value = (Owner->CurHP() > Target->CurHP())
				             ? 40.f
				             : 70.f;
		}
	}

	// Enemy CurrentHP < Player CurrentHP -> FireRate: 0.2 -> 0.4
	Owner->CurHP() > Target->CurHP() ? FireRate = 0.2f : FireRate = 0.4f;

	if (auto* Charge = States.FindByPredicate([](const auto& P) { return P.Key == EEnemyState::Charge; }))
	{
		// Enemy CurrentKi < MaxKi * 0.4 -> Add: 0.3
		// Enemy CurrentKi > MaxKi * 0.4 -> Subtract: 0.3
		if (Charge->Value < 10.f) // Charge Weight can't exceed 10.f
		{
			if (Owner->CurKi() <= Target->MaxKi(0.4f))
			{
				Charge->Value = Charge->Value + 2.f;
				if (Charge->Value >= 10.f) Charge->Value = 0.f;
			}
			else if (Owner->CurKi() > Target->MaxKi(0.4f))
			{
				Charge->Value = Charge->Value - 2.f;
				if (Charge->Value <= 0.f) Charge->Value = 0.f; // Initialize if Charge Weight is under 0.f
			}
		}

		// Enemy CurrentKi < MaxKi * 0.2
		if (Owner->CurKi() < Target->MaxKi(0.2f))
		{
			Charge->Value = 10.f;
		}

		// TargetDistance > MeleeDistance
		if (TargetDistance > MeleeDistance)
		{
			Charge->Value = Charge->Value + 2.f;
			if (Charge->Value >= 10.f)
				Charge->Value = 0.f;
		}

		// Enemy CurrentKi > MaxKi * 0.6
		if (Owner->CurKi() >= Target->MaxKi(0.6f))
		{
			Charge->Value = 2.f;
		}

		// Enemy CurrentKi == MaxKi * 0.8
		if (Owner->CurKi() >= Target->MaxKi(0.8f))
		{
			Charge->Value = 0.f;
		}
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

	Owner->RushAttackSystem->OnAttack();

	// 대상의 비행 상태에 따라 소유자의 이동 모드를 동기화합니다.
	const EMovementMode OwnerMoveMode = Owner->GetCharacterMovement()->MovementMode;
	const EMovementMode TargetMoveMode = Target->GetCharacterMovement()->MovementMode;

	const bool bOwnerIsFlying = (OwnerMoveMode == EMovementMode::MOVE_Flying);
	const bool bTargetIsFlying = (TargetMoveMode == EMovementMode::MOVE_Flying);

	// 소유자와 대상의 비행 상태가 다를 경우
	if (bOwnerIsFlying != bTargetIsFlying)
	{
		const EMovementMode ResultMode = bTargetIsFlying ? EMovementMode::MOVE_Flying : EMovementMode::MOVE_Walking;
		Owner->GetCharacterMovement()->SetMovementMode(ResultMode);
	}

	bActing = false;
}

void UEnemyFSM::Charge()
{
	Owner->ChargeKiSystem->ActivateEffect(true);
	float ChargeTime = FMath::RandRange(1.5f, 3.9f);
	FTimerHandle ChargeTimer;
	GetWorld()->GetTimerManager().SetTimer(ChargeTimer, [this]()
	{
		Owner->ChargeKiSystem->ActivateEffect(false);
	}, ChargeTime, false);
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
	if (!Owner->IsBlastShootEnable())
	{
		PRINT_STRING(TEXT("Enemy Is ShootBlastDisable!!!!"));
		return;
	}
	Owner->EnergyBlastShoot();

	//
	// Owner->UseBlast();
	// Owner->PlayTypeMontage(EAnimMontageType::Blast);
	//
	// FActorSpawnParameters Params;
	// Params.Owner = Owner;
	// Params.Instigator = Owner;
	//
	// GetWorld()->SpawnActor<AEnergyBlastActor>(
	// 	Owner->EnergyBlastFactory,
	// 	Owner->GetBodyPart(EBodyPartType::Hand_R)->GetComponentTransform(),
	// 	Params
	// );
}

void UEnemyFSM::SpawnEnergyBlastLoop(int32 Remaining)
{
	if (Remaining <= 0)
		return;

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
		Owner->DashSystem->NiagaraComp->Activate(Active);
		return;
	}
	Owner->DashSystem->NiagaraComp->Deactivate();
}

void UEnemyFSM::CheckToLand()
{
	FHitResult Hit;
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		Owner->GetActorLocation(),
		Owner->GetActorLocation() - FVector(0, 0, 35.f),
		ECC_WorldDynamic
	);
	if (Hit.GetActor()->ActorHasTag("Ground")) Owner->FlySystem->OnLand(Hit);
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
	if (Owner->GetCharacterMovement()->MovementMode != MOVE_Flying)
		Owner->GetCharacterMovement()->
		       SetMovementMode(MOVE_Flying);

	CumulativeDistance += MoveSpeed * GetWorld()->GetDeltaSeconds();
	float t = FindT(CumulativeDistance);
	FVector Pos = Bezier(OriginLocation, CenterControlPoint, Destination, t);

	if (!bPlayingMontage)
	{
		AnimInstance->Montage_Play(DashMontage);
		Owner->DashSystem->DashWindComp->Activate();
		bPlayingMontage = true;
	}
	// Approach distance tolerance
	if (ArcLength[Samples] < CumulativeDistance)
	{
		bMoving = false;
		CumulativeDistance = 0;
		ActivateDashVFX(false);
		CheckToLand();
		AnimInstance->Montage_Stop(0.2f, DashMontage);
		bPlayingMontage = false;
		Owner->DashSystem->DashWindComp->DeactivateImmediate();
		return;
	}
	Owner->SetActorLocation(Pos, true);
	Owner->DashSystem->NiagaraComp->SetWorldRotation(Owner->GetActorForwardVector().Rotation());
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
	OriginLocation = Owner->GetActorLocation();
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
