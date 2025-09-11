// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "APlayerActor.h"
#include "AEnemyActor.h"

#include "Core/Macro.h"

#include "DragonBallSZ.h"
#include "UDBSZEventManager.h"

#include "UStatSystem.h"
#include "UHitStopSystem.h"
#include "URushAttackSystem.h"
#include "UDashSystem.h"
#include "UFlySystem.h"
#include "UKnockbackSystem.h"
// #include "TimerManager.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetMathLibrary.h"

APlayerActor::APlayerActor()
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
	
	bUseControllerRotationYaw = true;
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->bOrientRotationToMovement = false;

		// 플라잉 모드 미끄러짐 방지 설정
		// 높은 값으로 설정하여 즉시 멈추도록 함
		Move->BrakingDecelerationFlying = 4096.0f; // 기본값 0.0f
    
		// 추가적으로 마찰력도 조정 가능
		Move->BrakingFriction = 4.0f; // 기본값 0.0f
	}
}

void APlayerActor::BeginPlay()
{
	Super::BeginPlay();

	if ( AActor* FoundActor = UGameplayStatics::GetActorOfClass( GetWorld(), AEnemyActor::StaticClass() ) )
		TargetActor = Cast<AEnemyActor>(FoundActor);

	// ActorComponent 초기화
	StatSystem->InitStat(true);
	RushAttackSystem->InitSystem(this);
	RushAttackSystem->SetDamage( StatSystem->Damage );
	KnockbackSystem->InitSystem(this);
	DashSystem->InitSystem(this, DashNiagaraSystem);
	FlySystem->InitSystem(this, BIND_DYNAMIC_DELEGATE(FEndCallback, this, APlayerActor, OnFlyEnd));
	HitStopSystem->InitSystem(this);

	// 이벤트 매니저를 통한 이벤트 등록및 제어
	EventManager = UDBSZEventManager::Get(GetWorld());
	EventManager->OnDash.AddDynamic(this, &APlayerActor::OnDash);
	EventManager->OnTeleport.AddDynamic(this, &APlayerActor::OnTeleport);
	EventManager->OnAttack.AddDynamic(this, &APlayerActor::OnAttack);
	EventManager->OnSpecialAttack.AddDynamic(this, &APlayerActor::OnSpecialAttack);
	EventManager->OnGuard.AddDynamic(this, &APlayerActor::OnGuard);
	EventManager->OnAvoid.AddDynamic(this, &APlayerActor::OnAvoid);
	EventManager->OnPowerCharge.AddDynamic(this, &APlayerActor::OnPowerCharge);

	EventManager->SendUpdateHealth(true, StatSystem->CurHP, StatSystem->MaxHP);
}

void APlayerActor::OnDash(AActor* Target, bool IsDashing)
{
	if ( this != Target )
		return;
	const TCHAR* PrintMsg = IsDashing ? TEXT("Player Dashing Start") : TEXT("Player Dashing Complete");
	PRINTLOG(TEXT("%s"), PrintMsg);
}

void APlayerActor::OnTeleport(AActor* Target)
{
	if ( this != Target )
		return;

	PRINTLOG(TEXT("OnTeleport"));
}

void APlayerActor::OnAttack(AActor* Target, int ComboCount)
{
	if ( this != Target )
		return;

	PRINTLOG(TEXT("ComboCount : %d"), ComboCount);
}

void APlayerActor::OnSpecialAttack(AActor* Target, int32 SpecialIndex)
{
	if ( this != Target )
		return;

	PRINTLOG(TEXT("OnSpecialAttack : %d"), SpecialIndex);
}

void APlayerActor::OnGuard(AActor* Target, bool bState)
{
	if ( this != Target )
		return;
	
	const TCHAR* PrintMsg = bState ? TEXT("Player Guard Start") : TEXT("Player Guard End");
	PRINTLOG(TEXT("%s"), PrintMsg);
}

void APlayerActor::OnAvoid(AActor* Target, bool bState)
{
	if ( this != Target )
		return;
	
	const TCHAR* PrintMsg = bState ? TEXT("Player Avoid Start") : TEXT("Player Avoid End");
	PRINTLOG(TEXT("%s"), PrintMsg);
}

void APlayerActor::OnPowerCharge(AActor* Target, bool bState)
{
	if ( this != Target )
		return;
	
	const TCHAR* PrintMsg = bState ? TEXT("Player PowerCharge Start") : TEXT("Player PowerCharge End");
	PRINTLOG(TEXT("%s"), PrintMsg);
}

void APlayerActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if ( RushAttackSystem->ShouldLookAtTarget())
        this->OnLookTarget();
}

void APlayerActor::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (FlySystem)
		FlySystem->OnLand(Hit);
}

bool APlayerActor::IsControlEnable_Implementation()
{
	if ( IsHit )
		return false;

	if ( StatSystem->IsDead )
		return false;

	return true;
}

bool APlayerActor::IsMoveEnable_Implementation()
{
	if ( !IsControlEnable() )
		return false;

	if ( IsAttackIng() )
		return false;

	return true;
}

bool APlayerActor::IsAttackEnable_Implementation()
{
	if ( IsHit )
		return false;

	return true;
}

bool APlayerActor::IsHiting_Implementation()
{
	return IsHit;
}

bool APlayerActor::IsAttackIng_Implementation()
{
	return RushAttackSystem->IsAttackIng();
}


bool APlayerActor::IsInSight(const AActor* Other) const
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

void APlayerActor::OnLookTarget_Implementation()
{
	if (!TargetActor)
		return;

	const FVector TargetLoc = TargetActor->GetActorLocation();
	const FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargetLoc);
	const FRotator NewRot(0.f, LookAt.Yaw, 0.f);

	SetActorRotation(NewRot);
}

void APlayerActor::OnFlyEnd_Implementation()
{
	DashSystem->ActivateEffect(false);
}

void APlayerActor::OnRestoreAvoid()
{
	EventManager->SendAvoid(this, false);
}

void APlayerActor::SetFlying()
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	
	MoveComp->SetMovementMode(MOVE_Flying);

	this->bUseControllerRotationYaw = true;
	this->bUseControllerRotationPitch = true;
	MoveComp->bOrientRotationToMovement = false;
}

void APlayerActor::Cmd_Move_Implementation(const FVector2D& Axis)
{
	if ( !IsMoveEnable() )
		return;
	
	// if (Controller)
	// {
	// 	// Move By Control
	// 	// const FRotator ControlRot = Controller->GetControlRotation();
	// 	// const FRotator YawRot(0.f, ControlRot.Yaw, 0.f);
	// 	// const FVector Forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
	// 	// const FVector Right   = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);
	// }

	{
		// Move By Actor
		const FVector Forward = GetActorForwardVector();
		const FVector Right   = GetActorRightVector();

		AddMovementInput(Forward, Axis.Y);
		AddMovementInput(Right,   Axis.X);
	}
}

void APlayerActor::Cmd_Look_Implementation(const FVector2D& Axis)
{
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void APlayerActor::Cmd_Jump_Implementation()
{
	if ( !IsMoveEnable() )
		return;

	FlySystem->OnJump();
}

void APlayerActor::Cmd_Dash_Implementation()
{
	if ( !IsMoveEnable() )
		return;

	PRINTINFO();
}

void APlayerActor::Cmd_Landing_Implementation()
{
	if ( !IsControlEnable() )
		return;

	FHitResult HitResult;
	FlySystem->OnLand(HitResult);
}

void APlayerActor::Cmd_ChargeKi_Implementation(bool bPressed)
{
	if ( !IsControlEnable() )
		return;

	EventManager->SendPowerCharge(this, bPressed);
}

void APlayerActor::Cmd_Guard_Implementation(bool bPressed)
{
	if ( !IsControlEnable() )
		return;

	EventManager->SendGuard(this, bPressed);
}

void APlayerActor::Cmd_Vanish_Implementation()
{
	if ( !IsControlEnable() )
		return;
	
	EventManager->SendAvoid(this, true);

	GetWorld()->GetTimerManager().ClearTimer(AvoidTimer);
	GetWorld()->GetTimerManager().SetTimer(
		AvoidTimer, this, &APlayerActor::OnRestoreAvoid, AvoidTime, false
	);
}

void APlayerActor::Cmd_RushAttack_Implementation()
{
	if ( !IsControlEnable() )
		return;

	RushAttackSystem->OnAttack();
}

void APlayerActor::Cmd_EnergyBlast_Implementation()
{
	if ( !IsControlEnable() )
		return;

	PRINTINFO();
}

void APlayerActor::Cmd_Kamehameha_Implementation()
{
	if ( !IsControlEnable() )
		return;

	EventManager->SendSpecialAttack(this, 1);
}


