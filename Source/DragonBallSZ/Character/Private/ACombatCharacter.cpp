// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "ACombatCharacter.h"

#include "UStatSystem.h"
#include "UHitStopSystem.h"
#include "URushAttackSystem.h"
#include "UDashSystem.h"
#include "UFlySystem.h"
#include "UKnockbackSystem.h"

#include "GameEvent.h"
#include "UDBSZEventManager.h"
#include "DragonBallSZ.h"

#include "Components/ArrowComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"


ACombatCharacter::ACombatCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, 270.0f, 0.0f));

	StatSystem			= CreateDefaultSubobject<UStatSystem>(TEXT("StatSystem"));
	HitStopSystem		= CreateDefaultSubobject<UHitStopSystem>(TEXT("HitStopSystem"));
	KnockbackSystem		= CreateDefaultSubobject<UKnockbackSystem>(TEXT("KnockbackSystem"));
	RushAttackSystem	= CreateDefaultSubobject<URushAttackSystem>(TEXT("RushAttackSystem"));
	DashSystem			= CreateDefaultSubobject<UDashSystem>(TEXT("DashSystem"));
	FlySystem			= CreateDefaultSubobject<UFlySystem>(TEXT("FlySystem"));
	
	LeftHandComp = CreateDefaultSubobject<UArrowComponent>(TEXT("LeftHandComp"));
	LeftHandComp->SetupAttachment(GetMesh(), TEXT("hand_l"));

	RightHandComp = CreateDefaultSubobject<UArrowComponent>(TEXT("RightHandComp"));
	RightHandComp->SetupAttachment(GetMesh(), TEXT("hand_r"));
	RightHandComp->SetRelativeRotation(FRotator(0, -180.f, 0.f));

	LeftFootComp = CreateDefaultSubobject<UArrowComponent>(TEXT("LeftFootComp"));
	LeftFootComp->SetupAttachment(GetMesh(), TEXT("foot_l"));
	LeftFootComp->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));

	RightFootComp = CreateDefaultSubobject<UArrowComponent>(TEXT("RightFootComp"));
	RightFootComp->SetupAttachment(GetMesh(), TEXT("foot_r"));
}

void ACombatCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (auto EventManager = UDBSZEventManager::Get(GetWorld()))
		EventManager->OnMessage.AddDynamic(this, &ACombatCharacter::OnRecvMessage );
}

void ACombatCharacter::OnRecvMessage(FString InMsg)
{
	if ( InMsg == GameEvent::CombatStart )
		bIsCombatStart = true;
	else if ( InMsg == GameEvent::PlayerWin )
	{
		bIsCombatResult = true;
		bIsWinner = this->IsPlayer();

		PRINT_STRING(TEXT("WINNER IS PLAYER"));
	}
	else if ( InMsg == GameEvent::EnemyWin )
	{
		bIsCombatResult = true;
		bIsWinner = this->IsEnemy();

		PRINT_STRING(TEXT("ENEMY IS PLAYER"));
	}
}

void ACombatCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// if ( RushAttackSystem->ShouldLookAtTarget())
	// 	this->OnLookTarget();
}

void ACombatCharacter::OnLookTarget_Implementation()
{
	if (!TargetActor)
		return;

	const FVector TargetLoc = TargetActor->GetActorLocation();
	const FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargetLoc);
	const FRotator NewRot(0.f, LookAt.Yaw, 0.f);

	SetActorRotation(NewRot);
}

void ACombatCharacter::OnFlyEnd_Implementation()
{
	DashSystem->ActivateEffect(false);
}

void ACombatCharacter::SetFlying()
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->SetMovementMode(MOVE_Flying);

	this->bUseControllerRotationYaw = true;
	this->bUseControllerRotationPitch = true;
	MoveComp->bOrientRotationToMovement = false;
}

void ACombatCharacter::SetFallingToWalk()
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->SetMovementMode( EMovementMode::MOVE_Falling );

	this->bUseControllerRotationYaw = false;
	this->bUseControllerRotationPitch = false;
	MoveComp->bOrientRotationToMovement = true;
}

void ACombatCharacter::RecoveryMovementMode(const EMovementMode InMovementMode)
{
	if ( InMovementMode == MOVE_None)
		return;
	
	auto Movement = this->GetCharacterMovement();

	if ( InMovementMode == MOVE_Flying )
	{
		Movement->SetMovementMode( EMovementMode::MOVE_Flying );
		this->bUseControllerRotationYaw = true;
		this->bUseControllerRotationPitch = true;
		Movement->bOrientRotationToMovement = false;
	}
	else
	{
		Movement->SetMovementMode( InMovementMode );
		this->bUseControllerRotationYaw = false;
		this->bUseControllerRotationPitch = false;
		Movement->bOrientRotationToMovement = true;
	}
}

bool ACombatCharacter::IsControlEnable_Implementation()
{
	if ( IsCombatStart() == false || IsCombatResult())
	{
		// 전투 시작 전
		// 전투 결과 후
		return false;
	}
	
	if ( IsHit )
		return false;

	if ( StatSystem->IsDead )
		return false;

	return true;
}

bool ACombatCharacter::IsMoveEnable_Implementation()
{
	if ( !IsControlEnable() )
		return false;

	if ( IsAttackIng() )
		return false;

	return true;
}

bool ACombatCharacter::IsAttackEnable_Implementation()
{
	if ( IsHit )
		return false;

	return true;
}

bool ACombatCharacter::IsHitting_Implementation()
{
	return IsHit;
}

bool ACombatCharacter::IsAttackIng_Implementation()
{
	return RushAttackSystem->IsAttackIng();
}

bool ACombatCharacter::IsInSight(const AActor* Other) const
{
	const FVector SelfLoc = GetActorLocation();
	const FVector OtherLoc = Other->GetActorLocation();
	const FVector ToOther = OtherLoc - SelfLoc;

	const float Dist = ToOther.Size();
	if (Dist > SightRange)
		return false;

	const FVector Fwd = GetActorForwardVector();
	const float CosHalfFOV = FMath::Cos(FMath::DegreesToRadians(SightHalfFOVDeg));
	const float CosAngle = FVector::DotProduct(Fwd, ToOther.GetSafeNormal());
	if (CosAngle < CosHalfFOV)
		return false;

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(PlayerSightLOS), false, this);

	const bool bBlocked = GetWorld()->LineTraceSingleByChannel(
		Hit,
		SelfLoc + FVector(0,0,50),
		OtherLoc + FVector(0,0,50),
		ECC_Visibility,
		Params
	);
    
	if (bBlocked && Hit.GetActor() != Other)
		return false;
	
	return true;
}