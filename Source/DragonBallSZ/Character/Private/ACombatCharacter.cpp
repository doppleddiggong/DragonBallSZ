// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "ACombatCharacter.h"

#include "UStatSystem.h"
#include "UHitStopSystem.h"
#include "URushAttackSystem.h"
#include "UDashSystem.h"
#include "UFlySystem.h"
#include "UKnockbackSystem.h"
#include "UChargeKiSystem.h"

#include "GameEvent.h"
#include "UDBSZEventManager.h"
#include "DragonBallSZ.h"
#include "EAnimMontageType.h"
#include "UDBSZDamageType.h"
#include "UDBSZVFXManager.h"
#include "UDBSZSoundManager.h"

#include "Components/ArrowComponent.h"
#include "Components/CapsuleComponent.h"
#include "Features/UCommonFunctionLibrary.h"
#include "Features/UDelayTaskManager.h"
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
	ChargeKiSystem		= CreateDefaultSubobject<UChargeKiSystem>(TEXT("ChargeKiSystem"));
	
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

	MeshComp = this->GetMesh();
	AnimInstance = MeshComp->GetAnimInstance();
	
	OnTakeAnyDamage.AddDynamic(this, &ACombatCharacter::OnDamage);

	EventManager = UDBSZEventManager::Get(GetWorld());
	EventManager->OnMessage.AddDynamic(this, &ACombatCharacter::OnRecvMessage );
 }

void ACombatCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (EventManager)
	{
		EventManager->OnMessage.RemoveDynamic(this, &ACombatCharacter::OnRecvMessage);
	}
}

bool ACombatCharacter::IsControlEnable_Implementation()
{
	if ( this->IsHolding() )
		return false;
	
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


UAnimMontage* ACombatCharacter::GetRandomHitAnim()
{
	return UCommonFunctionLibrary::GetRandomMontage(HitMontages);
}

UAnimMontage* ACombatCharacter::GetRandomBlastAnim()
{
	return UCommonFunctionLibrary::GetRandomMontage(BlastMontages);
}

void ACombatCharacter::OnRecvMessage(FString InMsg)
{
	if ( InMsg == GameEvent::CombatStart )
	{
		EventManager->SendUpdateHealth(IsPlayer(), StatSystem->CurHP, StatSystem->MaxHP);
		EventManager->SendUpdateKi(IsPlayer(), StatSystem->CurKi, StatSystem->MaxKi);

		bIsCombatStart = true;
	}
	else if ( InMsg == GameEvent::PlayerWin )
	{
		bIsCombatResult = true;
		bIsWinner = this->IsPlayer();
	
		// PRINTLOG(TEXT("WINNER IS PLAYER"));
	}
	else if ( InMsg == GameEvent::EnemyWin )
	{
		bIsCombatResult = true;
		bIsWinner = this->IsEnemy();

		// PRINTLOG(TEXT("ENEMY IS PLAYER"));
	}
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

void ACombatCharacter::OnDamage(
	AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	AController* InstigatedBy, AActor* DamageCauser )
{
	if ( DamagedActor != this)
		return;
	
	IsHit = true;

	const auto AttackPowerType = Cast<const UDBSZDamageType>(DamageType)->AttackPowerType;
	this->PlaySoundHit();

	UDBSZVFXManager::Get(GetWorld())->ShowVFXAttackType(
					AttackPowerType,
					GetActorLocation(),
					GetActorRotation(),
					FVector(1.f));
	
	bool IsDie = StatSystem->DecreaseHealth(Damage);

	EventManager->SendCameraShake(this, EAttackPowerType::Normal );
	EventManager->SendDamage(IsPlayer(), Damage);

	if ( ChargeKiSystem->IsActivateState() )
	{
		// 기 차지 캔슬!
		ChargeKiSystem->ActivateEffect(false);
	}
	
	
	if ( IsDie )
	{
		FName SendEventType = IsPlayer() ? GameEvent::EnemyWin : GameEvent::PlayerWin;
		EventManager->SendMessage( SendEventType.ToString() );

		this->PlaySoundWin();
		this->PlayTypeMontage(EAnimMontageType::Death);
		this->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);	
	}
	else
	{
		auto HitAnimMontage = GetRandomHitAnim();
		float HitEndTime = HitAnimMontage->GetPlayLength();

		this->PlayTargetMontage(HitAnimMontage);

		UDelayTaskManager::Get(this)->Delay(this, HitEndTime, [this](){
			IsHit = false;
		});
	}
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

bool ACombatCharacter::IsBlastShootEnable()
{
	if ( !IsControlEnable() )
		return false;

	const float Now = GetWorld()->GetTimeSeconds();
	const float NextAvailableTime = LastBlastShotTime + GetBlastShootDelay();
	if (NextAvailableTime > Now )
		return false;
	
	return StatSystem->CurKi > StatSystem->BlastNeedKi;
}

void ACombatCharacter::PlayTypeMontage(EAnimMontageType Type)
{
	UAnimMontage* AnimMontage = nullptr;

	switch (Type)
	{
	case EAnimMontageType::Death:
		AnimMontage = DeathMontage;
		break;
	case EAnimMontageType::Blast:
		AnimMontage = GetRandomBlastAnim();
		break;
	case EAnimMontageType::Kamehame:
		AnimMontage = KamehameMontage;
		break;
	case EAnimMontageType::Intro:
		AnimMontage = IntroMontage;
		break;
	case EAnimMontageType::Win:
		AnimMontage = WinMontage;
		break;
	case EAnimMontageType::ChargeKi:
		AnimMontage = ChargeKiMontage;
		break;
	}

	this->PlayTargetMontage(AnimMontage);
}

void ACombatCharacter::PlayTargetMontage(UAnimMontage* AnimMontage)
{
	if ( IsValid(AnimInstance) && IsValid(AnimMontage) )
	{
		AnimInstance->Montage_Play(
			AnimMontage,
			1.0f,
			EMontagePlayReturnType::MontageLength,
			0.f,
			true);
	}
	else
	{
		PRINTLOG(TEXT("Failed to PlayMontage"));
	}
}

void ACombatCharacter::StopTargetMontage(EAnimMontageType Type, float BlendInOutTime)
{
	UAnimMontage* AnimMontage = nullptr;

	switch (Type)
	{
	case EAnimMontageType::Death:
		AnimMontage = DeathMontage;
		break;
	case EAnimMontageType::Blast:
		AnimMontage = GetRandomBlastAnim();
		break;
	case EAnimMontageType::Kamehame:
		AnimMontage = KamehameMontage;
		break;
	case EAnimMontageType::Intro:
		AnimMontage = IntroMontage;
		break;
	case EAnimMontageType::Win:
		AnimMontage = WinMontage;
		break;
	case EAnimMontageType::ChargeKi:
		AnimMontage = ChargeKiMontage;
		break;
	}
	
	AnimInstance->Montage_Stop(BlendInOutTime, AnimMontage );
}


void ACombatCharacter::PlaySoundAttack()
{
	auto SoundType = IsPlayer() ? ESoundType::Goku_Attack : ESoundType::Vege_Attack;
	UDBSZSoundManager::Get(GetWorld())->PlaySound2D( SoundType );
}

void ACombatCharacter::PlaySoundHit()
{
	auto SoundType = IsPlayer() ? ESoundType::Goku_Hit : ESoundType::Vege_Hit;
	UDBSZSoundManager::Get(GetWorld())->PlaySound2D( SoundType );
}

void ACombatCharacter::PlaySoundJump()
{
	auto SoundType = IsPlayer() ? ESoundType::Goku_Jump : ESoundType::Vege_Jump;
	UDBSZSoundManager::Get(GetWorld())->PlaySound2D( SoundType );
}

void ACombatCharacter::PlaySoundTeleport()
{
	auto SoundType = IsPlayer() ? ESoundType::Goku_Teleport : ESoundType::Vege_Teleport;
	UDBSZSoundManager::Get(GetWorld())->PlaySound2D( SoundType );
}

void ACombatCharacter::PlaySoundWin()
{
	auto SoundType = IsPlayer() ? ESoundType::Goku_Win : ESoundType::Vege_Win;
	UDBSZSoundManager::Get(GetWorld())->PlaySound2D( SoundType );
}