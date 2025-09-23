// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "ACombatCharacter.h"

#include "AEnergyBlastActor.h"
#include "AKamehamehaActor.h"
#include "UStatSystem.h"
#include "UHitStopSystem.h"
#include "URushAttackSystem.h"
#include "UDashSystem.h"
#include "UFlySystem.h"
#include "UKnockbackSystem.h"
#include "UChargeKiSystem.h"

#include "GameEvent.h"
#include "GameColor.h"

#include "UCharacterData.h"
#include "UDBSZEventManager.h"
#include "DragonBallSZ.h"
#include "EAnimMontageType.h"
#include "UDBSZDamageType.h"
#include "UDBSZDataManager.h"
#include "UDBSZVFXManager.h"
#include "UDBSZSoundManager.h"

#include "Shared/FComponentHelper.h"

#include "Components/ArrowComponent.h"
#include "Components/CapsuleComponent.h"
#include "Features/UCommonFunctionLibrary.h"
#include "Features/UDelayTaskManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Materials/MaterialInstanceConstant.h"

#define CHARGEKI_WHITE_PATH TEXT("/Game/VFX/InGame/ChargeKi/M_WhiteOutline_Inst.M_WhiteOutline_Inst")

ACombatCharacter::ACombatCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	StatSystem			= CreateDefaultSubobject<UStatSystem>(TEXT("StatSystem"));
	HitStopSystem		= CreateDefaultSubobject<UHitStopSystem>(TEXT("HitStopSystem"));
	KnockbackSystem		= CreateDefaultSubobject<UKnockbackSystem>(TEXT("KnockbackSystem"));
	RushAttackSystem	= CreateDefaultSubobject<URushAttackSystem>(TEXT("RushAttackSystem"));
	DashSystem			= CreateDefaultSubobject<UDashSystem>(TEXT("DashSystem"));
	FlySystem			= CreateDefaultSubobject<UFlySystem>(TEXT("FlySystem"));
	ChargeKiSystem		= CreateDefaultSubobject<UChargeKiSystem>(TEXT("ChargeKiSystem"));
	
	LeftHandComp = CreateDefaultSubobject<UArrowComponent>(TEXT("LeftHandComp"));
	RightHandComp = CreateDefaultSubobject<UArrowComponent>(TEXT("RightHandComp"));
	LeftFootComp = CreateDefaultSubobject<UArrowComponent>(TEXT("LeftFootComp"));
	RightFootComp = CreateDefaultSubobject<UArrowComponent>(TEXT("RightFootComp"));

	if (auto LoadedAsset = FComponentHelper::LoadAsset<UMaterialInterface>(CHARGEKI_WHITE_PATH))
		OverlayMaterial = LoadedAsset;
}

void ACombatCharacter::BeginPlay()
{
	Super::BeginPlay();

	MoveComp = this->GetCharacterMovement();
	OverlayMID = UMaterialInstanceDynamic::Create(OverlayMaterial, this);

	this->SetFallingToWalk();
	
	OnTakeAnyDamage.AddDynamic(this, &ACombatCharacter::OnDamage);
	
	EventManager = UDBSZEventManager::Get(GetWorld());
	EventManager->OnMessage.AddDynamic(this, &ACombatCharacter::OnRecvMessage );
	EventManager->OnPowerCharge.AddDynamic(this, &ACombatCharacter::OnPowerCharge);
 }

void ACombatCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AnimInstance && bDelegatesBound)
		UnbindMontageDelegates(AnimInstance);
	bDelegatesBound = false;

	if (EventManager)
	{
		EventManager->OnMessage.RemoveDynamic(this, &ACombatCharacter::OnRecvMessage);
		EventManager->OnPowerCharge.RemoveDynamic(this, &ACombatCharacter::OnPowerCharge);
	}
	
	Super::EndPlay(EndPlayReason);
}


void ACombatCharacter::SetupCharacterFromType(const ECharacterType Type, const bool bIsAnother)
{
	this->CharacterType = Type;
	
	FCharacterAssetData Params;
	UDBSZDataManager::Get(GetWorld())->GetCharacterAssetData(Type, Params);

	if (Params.CharacterDataAsset.IsValid())
		this->CharacterData = Params.CharacterDataAsset.Get();
	else
		this->CharacterData = Params.CharacterDataAsset.LoadSynchronous();

	MeshComp = GetMesh();
    if ( IsValid(MeshComp) )
    {
        if (CharacterData->MeshData.IsValid())
			MeshComp->SetSkeletalMesh(CharacterData->MeshData.Get());
        else
            MeshComp->SetSkeletalMesh(CharacterData->MeshData.LoadSynchronous());

    	MeshComp->EmptyOverrideMaterials();
    	for (int32 i = 0; i < CharacterData->MaterialArray.Num(); ++i)
    	{
    		UMaterialInstanceConstant* MaterialInst = nullptr;
    		if (CharacterData->MaterialArray[i].IsValid())
    			MaterialInst = CharacterData->MaterialArray[i].Get();
    		else
    			MaterialInst = CharacterData->MaterialArray[i].LoadSynchronous();

    		if (MaterialInst)
    		{
    			auto DynamicMat = UMaterialInstanceDynamic::Create(MaterialInst, MeshComp);
    			if (DynamicMat)
    			{
    				// 1번이 Cloth
    				if (i == 1)
    					DynamicMat->SetScalarParameterValue(FName("AnotherSide"), bIsAnother ? 1.0f : 0.0f);
    				MeshComp->SetMaterial(i, DynamicMat);
    			}
    		}
    	}
    	
    	MeshComp->SetRelativeLocation( CharacterData->RelativeLocation);
    	MeshComp->SetRelativeRotation(  CharacterData->RelativeRotator );
    	MeshComp->SetRelativeScale3D( CharacterData->RelativeScale );
    	
        if (CharacterData->AnimBluePrint.IsValid())
            MeshComp->SetAnimInstanceClass(CharacterData->AnimBluePrint.Get());
        else
            MeshComp->SetAnimInstanceClass(CharacterData->AnimBluePrint.LoadSynchronous());

    	auto Rule = FAttachmentTransformRules::SnapToTargetNotIncludingScale;
    	
    	LeftHandComp->AttachToComponent(MeshComp, Rule, CharacterData->LeftHandSocketName);
    	RightHandComp->AttachToComponent(MeshComp, Rule, CharacterData->RightHandSocketName);
    	LeftFootComp->AttachToComponent(MeshComp, Rule, CharacterData->LeftFootSocketName);
    	RightFootComp->AttachToComponent(MeshComp, Rule, CharacterData->RightFootSocketName);

    	MeshComp->SetOverlayMaterial(OverlayMID);

    	AnimInstance = MeshComp->GetAnimInstance();
    	BindMontageDelegates(AnimInstance);
    }

	// AsyncLoad
	CharacterData->LoadHitMontage(HitMontages);
	CharacterData->LoadDeathMontage(DeathMontage);
	CharacterData->LoadBlastMontage(BlastMontages);
	CharacterData->LoadChargeKiMontage(ChargeKiMontage);
	CharacterData->LoadKamehameMontage(KamehameMontage);
	CharacterData->LoadIntroMontage(IntroMontage);
	CharacterData->LoadWinMontage(WinMontage);

	CharacterData->LoadDashVFX(DashVFX);
	CharacterData->LoadChargeKiVFX(ChargeKiVFX);

	CharacterData->LoadEnergyBlast(EnergyBlastFactory);
	CharacterData->LoadKamehame(KamehamehaFactory);
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

FVector ACombatCharacter::GetKamehameHandLocation() const
{
	const FVector LeftHandLoc = LeftHandComp->GetComponentLocation();
	const FVector RightHandLoc = RightHandComp->GetComponentLocation();

	return (LeftHandLoc + RightHandLoc) / 2.0f;
}

void ACombatCharacter::SetOverlayMID(const FLinearColor InColor, const float InValue)
{
	if ( IsValid(OverlayMID ))
	{
		OverlayMID->SetVectorParameterValue(TEXT("Color"), InColor);
		OverlayMID->SetScalarParameterValue(TEXT("Opacity"), InValue);
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

	if ( StatSystem->IsDead() )
		return false;

	return true;
}

bool ACombatCharacter::IsMoveEnable_Implementation()
{
	if ( !IsControlEnable() )
		return false;

	if ( IsAttackIng() )
		return false;

	if ( IsChargeKi() )
		return false;

	return true;
}

bool ACombatCharacter::IsAttackEnable_Implementation()
{
	if ( IsHit )
		return false;

	return true;
}

bool ACombatCharacter::IsDead_Implementation()
{
	return StatSystem->IsDead();
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
		EventManager->SendUpdateHealth(IsPlayer(), StatSystem->GetCurHP(), StatSystem->GetMaxHP());
		EventManager->SendUpdateKi(IsPlayer(), StatSystem->GetCurKi(), StatSystem->GetMaxKi());

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

void ACombatCharacter::OnPowerCharge(AActor* Target, bool bState)
{
	if ( this != Target )
		return;

	// OverlayWhite ON / OFF
	SetOverlayMID(GameColor::White, bState ? 1.0f : 0.0f);
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
	if ( IsShootKamehame() )
	{
		// 카메하메파 준비 동작중이므로, 무적 상태
		return;
	}
	
	if ( DamagedActor != this)
		return;
	
	IsHit = true;

	EAttackPowerType AttackPowerType = EAttackPowerType::Normal;
	if (const UDBSZDamageType* DBSZDamageType = Cast<const UDBSZDamageType>(DamageType))
	{
		AttackPowerType = DBSZDamageType->AttackPowerType;
	}
	this->PlaySoundHit();

	UDBSZVFXManager::Get(GetWorld())->ShowVFXAttackType(
					AttackPowerType,
					GetActorLocation(),
					GetActorRotation(),
					FVector(1.f));
	
	const bool IsDie = StatSystem->DecreaseHealth(Damage);

	EventManager->SendForceCameraShake(AttackPowerType);
	EventManager->SendDamage(IsPlayer(), Damage);

	RushAttackSystem->ResetComboCount();
	
	if ( ChargeKiSystem->IsActivateState() )
	{
		// 기 차지 캔슬!
		ChargeKiSystem->ActivateEffect(false);
	}
	
	if ( IsShootKamehame() || IsValid(KamehamehaActor) )
	{
		// 카메하메 파 캔슬!
		this->ClearKamehame();
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
		float HitEndTime = HitAnimMontage->GetPlayLength();// + HitEndOffset;


		if ( MoveComp->MovementMode != EMovementMode::MOVE_None )
		{
			PrevMoveMode = MoveComp->MovementMode;
			MoveComp->DisableMovement();
		}
		
		this->PlayTargetMontage(HitAnimMontage);

		if ( auto DelayManager = UDelayTaskManager::Get(this) )
		{
			DelayManager->CancelAll(this);
			DelayManager->Delay(this, HitEndTime*0.75f, [this](){
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

void ACombatCharacter::EnergyBlastShoot()
{
	EventManager->SendCameraShake(this, EAttackPowerType::Small );
	this->PlaySoundAttack(EAttackPowerType::Small);
	
	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this;

	this->UseBlast();
	this->PlayTypeMontage(EAnimMontageType::Blast);
	LastBlastShotTime = GetWorld()->GetTimeSeconds();

	GetWorld()->SpawnActor<AEnergyBlastActor>(
		EnergyBlastFactory,
		this->GetBodyPart(EBodyPartType::Hand_R)->GetComponentTransform(),
		Params
	);
}

void ACombatCharacter::KamehameShoot()
{
	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this;
	
	KamehamehaActor = GetWorld()->SpawnActor<AKamehamehaActor>(
		KamehamehaFactory,
		this->GetActorTransform(),
		Params
	);

	if ( IsValid(KamehamehaActor))
	{
		this->SetShootKamehame(true, KamehamehaActor);
		
		KamehamehaActor->StartKamehame(this, TargetActor);
	}
}

void ACombatCharacter::ClearKamehame()
{
	if ( IsValid(KamehamehaActor) )
	{
		KamehamehaActor->ClearKamehame();
		this->SetShootKamehame(false, nullptr);
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
	
	return StatSystem->IsBlastShotEnable();
}


bool ACombatCharacter::IsKamehameEnable_Implementation()
{
	if ( !IsControlEnable() )
		return false;

	return StatSystem->IsKamehameEnable();
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

void ACombatCharacter::PlaySoundAttack( EAttackPowerType Type )
{
	switch ( Type )
	{
		case EAttackPowerType::Small:
			{
				UDBSZSoundManager::Get(GetWorld())->PlaySound2D( CharacterData->SoundAttack01 );
			}
			break;

		case EAttackPowerType::Normal:
			{
				UDBSZSoundManager::Get(GetWorld())->PlaySound2D( CharacterData->SoundAttack02 );
			}
			break;

		case EAttackPowerType::Large:
			{
				UDBSZSoundManager::Get(GetWorld())->PlaySound2D( CharacterData->SoundAttack03 );
			}
			break;

		case EAttackPowerType::Huge:
			{
				UDBSZSoundManager::Get(GetWorld())->PlaySound2D( CharacterData->SoundAttack04 );
			}
			break;

		default:
			{
				UDBSZSoundManager::Get(GetWorld())->PlaySound2D( CharacterData->SoundAttack01 );
			}
			break;
	}
}

void ACombatCharacter::PlaySoundHit()
{
	UDBSZSoundManager::Get(GetWorld())->PlaySound2D( CharacterData->SoundHit );
}

void ACombatCharacter::PlaySoundJump()
{
	UDBSZSoundManager::Get(GetWorld())->PlaySound2D( CharacterData->SoundJump );
}

void ACombatCharacter::PlaySoundTeleport()
{
	UDBSZSoundManager::Get(GetWorld())->PlaySound2D( CharacterData->SoundTeleport );
}

void ACombatCharacter::PlaySoundWin()
{	
	UDBSZSoundManager::Get(GetWorld())->PlaySound2D( CharacterData->SoundWin );
}

void ACombatCharacter::PlayKamehame()
{	
	UDBSZSoundManager::Get(GetWorld())->PlaySound2D( CharacterData->SoundKamehame  );
}

void ACombatCharacter::PlayKamehamePa()
{	
	UDBSZSoundManager::Get(GetWorld())->PlaySound2D( CharacterData->SoundKamehamePa );
}

