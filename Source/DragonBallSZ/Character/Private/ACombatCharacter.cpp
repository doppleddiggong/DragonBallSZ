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
	MoveComp = this->GetCharacterMovement();

	AnimInstance = MeshComp->GetAnimInstance();
	
	OnTakeAnyDamage.AddDynamic(this, &ACombatCharacter::OnDamage);

	EventManager = UDBSZEventManager::Get(GetWorld());
	EventManager->OnMessage.AddDynamic(this, &ACombatCharacter::OnRecvMessage );


	BindMontageDelegates(AnimInstance);
 }

void ACombatCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AnimInstance && bDelegatesBound)
		UnbindMontageDelegates(AnimInstance);
	bDelegatesBound = false;

	if (EventManager)
		EventManager->OnMessage.RemoveDynamic(this, &ACombatCharacter::OnRecvMessage);
	
	Super::EndPlay(EndPlayReason);
}

void ACombatCharacter::BindMontageDelegates(UAnimInstance* Anim)
{
	if (!Anim || bDelegatesBound)
		return;

	// 중복 방지용으로 먼저 제거
	Anim->OnPlayMontageNotifyBegin.RemoveDynamic(this, &ACombatCharacter::OnMontageNotifyBegin);
	Anim->OnPlayMontageNotifyBegin.AddDynamic(this, &ACombatCharacter::OnMontageNotifyBegin);
	bDelegatesBound = true;
}

void ACombatCharacter::UnbindMontageDelegates(UAnimInstance* Anim)
{
	if (!Anim || !bDelegatesBound)
		return;

	Anim->OnPlayMontageNotifyBegin.RemoveDynamic(this, &ACombatCharacter::OnMontageNotifyBegin);

	bDelegatesBound = false;
}


void ACombatCharacter::OnMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& Payload)
{
	UE_LOG(LogTemp, Warning, TEXT("Notify Fired: %s"), *NotifyName.ToString());

	if ( NotifyName == GameEvent::KameShoot )
		EventManager->SendMessage(GameEvent::KameShoot.ToString());
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
	
	const bool IsDie = StatSystem->DecreaseHealth(Damage);

	EventManager->SendCameraShake(this, EAttackPowerType::Normal );
	EventManager->SendDamage(IsPlayer(), Damage);

	RushAttackSystem->ResetComboCount();
	
	if ( ChargeKiSystem->IsActivateState() )
	{
		// 기 차지 캔슬!
		ChargeKiSystem->ActivateEffect(false);
	}
	
	
	if ( IsDie )
	{
		FName SendEventType = IsPlayer() ? GameEvent::EnemyWin : GameEvent::PlayerWin;
		EventManager->SendMessage( SendEventType.ToString() );

		// this->PlaySoundWin();
		this->PlayTypeMontage(EAnimMontageType::Death);
		this->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);	
	}
	else
	{
		auto HitAnimMontage = GetRandomHitAnim();
		float HitEndOffset = 0.5f;
		float HitEndTime = HitAnimMontage->GetPlayLength() + HitEndOffset;


		if ( MoveComp->MovementMode != EMovementMode::MOVE_None )
		{
			PrevMoveMode = MoveComp->MovementMode;
			MoveComp->DisableMovement();
		}
		
		this->PlayTargetMontage(HitAnimMontage);

		if ( auto DelayManager = UDelayTaskManager::Get(this) )
		{
			DelayManager->CancelAll(this);
			DelayManager->Delay(this, HitEndTime, [this](){
				IsHit = false;
				MoveComp->SetMovementMode(PrevMoveMode);
			});
		}
	}
}

void ACombatCharacter::SetFlying()
{
	MoveComp->SetMovementMode(MOVE_Flying);
	PrevMoveMode = EMovementMode::MOVE_Flying;

	this->bUseControllerRotationYaw = true;
	this->bUseControllerRotationPitch = true;
	MoveComp->bOrientRotationToMovement = false;
}

void ACombatCharacter::SetFallingToWalk()
{
	MoveComp->SetMovementMode( EMovementMode::MOVE_Falling );
	PrevMoveMode = EMovementMode::MOVE_Falling;

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


bool ACombatCharacter::IsKamehameEnable_Implementation()
{
	if ( !IsControlEnable() )
		return false;

	return StatSystem->CurKi > StatSystem->KamehameNeedKi;
}

void ACombatCharacter::PlayTypeMontage(const EAnimMontageType Type)
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

void ACombatCharacter::StopTargetMontage(const EAnimMontageType Type, const float BlendInOutTime)
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
	auto SoundType = IsPlayer() ? ESoundType::Vege_Win : ESoundType::Goku_Win;
	UDBSZSoundManager::Get(GetWorld())->PlaySound2D( SoundType );
}