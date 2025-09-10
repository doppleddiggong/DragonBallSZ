// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "URushAttackSystem.h"

#include "AEnemyActor.h"
#include "APlayerActor.h"

#include "UDBSZEventManager.h"
#include "UDBSZDataManager.h"
#include "TimerManager.h"

#include "Components/CapsuleComponent.h"
#include "Components/ArrowComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

URushAttackSystem::URushAttackSystem()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void URushAttackSystem::BeginPlay()
{
	Super::BeginPlay();
}

void URushAttackSystem::EndPlay(const EEndPlayReason::Type Reason)
{
	if (AnimInstance && bDelegatesBound)
		UnbindMontageDelegates(AnimInstance);

	bDelegatesBound = false;

	Super::EndPlay(Reason);
}

void URushAttackSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsDashing)
	{
		ElapsedTime += DeltaTime;
        
		const float Alpha = FMath::Clamp(ElapsedTime / DashDuration, 0.0f, 1.0f);
		const FVector Location = FMath::Lerp(DashStartLoc, DashTargetLoc, Alpha);
        
		Owner->SetActorLocation(Location, true);
        
		const float Distance = FVector::Dist(Owner->GetActorLocation(), DashTargetLoc);
		if ( Distance <= AttackRange || ElapsedTime >= DashDuration )
			OnDashCompleted();
	}

	if ( bIsAttacking || bIsDashing )
		OnLookTarget();
}

void URushAttackSystem::BindMontageDelegates(UAnimInstance* Anim)
{
	if (!Anim || bDelegatesBound)
		return;

	// 중복 방지용으로 먼저 제거
	Anim->OnPlayMontageNotifyBegin.RemoveDynamic(this, &URushAttackSystem::OnMontageNotifyBegin);
	Anim->OnMontageEnded          .RemoveDynamic(this, &URushAttackSystem::OnMontageEnded);

	Anim->OnPlayMontageNotifyBegin.AddDynamic(this, &URushAttackSystem::OnMontageNotifyBegin);
	Anim->OnMontageEnded          .AddDynamic(this, &URushAttackSystem::OnMontageEnded);

	bDelegatesBound = true;
}

void URushAttackSystem::UnbindMontageDelegates(UAnimInstance* Anim)
{
	if (!Anim || !bDelegatesBound)
		return;

	Anim->OnPlayMontageNotifyBegin.RemoveDynamic(this, &URushAttackSystem::OnMontageNotifyBegin);
	Anim->OnMontageEnded          .RemoveDynamic(this, &URushAttackSystem::OnMontageEnded);

	bDelegatesBound = false;
}

void URushAttackSystem::OnMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& Payload)
{
	bIsAttacking = false;
	
	ComboCount++;
	if ( ComboCount > AttackMontages.Num()-1 )
		ComboCount = 0;
	
	ResetByHit();
}

void URushAttackSystem::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (bInterrupted)
	{
		ResetByHit();
		return;
	}

	if (AttackMontages.Num() > 0)
		ComboCount = (ComboCount + 1) % AttackMontages.Num();
}

void URushAttackSystem::InitSystem(APlayerActor* InOwner)
{
	this->Owner = InOwner;

	MeshComp = Owner->GetMesh();
	AnimInstance = MeshComp->GetAnimInstance();
	MoveComp = Owner->GetCharacterMovement();

	this->Target = Owner->TargetActor;
	TargetMoveComp = Target->GetCharacterMovement();

	BindMontageDelegates(AnimInstance);
}

void URushAttackSystem::OnLookTarget()
{
	if (!Owner || !Target)
		return;

	const FVector TargetLoc = Target->GetActorLocation();
	const FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(Owner->GetActorLocation(), TargetLoc);
	const FRotator NewRot(0.f, LookAt.Yaw, 0.f);
	Owner->SetActorRotation(NewRot);
}

void URushAttackSystem::OnDashCompleted()
{
	bIsDashing = false;
	AnimInstance->Montage_Stop(0.1f, DashMontages);

	if (Owner && !Owner->IsHit)
	{
		MoveComp->SetMovementMode(PrevMovementMode);
		PlayMontage(PendingMontageIndex);
	}
}

void URushAttackSystem::OnAttack()
{
    if (Owner->IsHit || bIsAttacking)
        return;

	const FVector OwnerLoc = Owner->GetActorLocation();
	const FVector TargetLoc = Target->GetActorLocation();
	const float Dist = FVector::Dist(OwnerLoc, TargetLoc);

	if (Dist >= TeleportRange)
	{
		TeleportToTarget(ComboCount);
	}
	else if (!bIsDashing)
	{
		DashToTarget(ComboCount);
	}
	else
	{
		PlayMontage(ComboCount);
	}
}

void URushAttackSystem::StartAttackTrace()
{
	GetWorld()->GetTimerManager().SetTimer(
		TraceTimeHandler,
		this,
		&URushAttackSystem::AttackTrace,
		0.01f,
		false
	);
}

void URushAttackSystem::StopAttackTrace()
{
    UKismetSystemLibrary::K2_ClearAndInvalidateTimerHandle(this, TraceTimeHandler);
}

void URushAttackSystem::AttackTrace()
{
	FVector Start, End;
	
	GetBodyLocation( Owner->GetBodyPart(AttackPart[ComboCount]), Start, End );
	AttackSphereTrace( Start, End, Damage, Owner);
}

void URushAttackSystem::GetBodyLocation(USceneComponent* SceneComp, FVector& OutStart, FVector& OutEnd) const
{
	if (!SceneComp)
	{
		OutStart = FVector::ZeroVector;
		OutEnd   = FVector::ZeroVector;
		return;
	}

	OutStart = SceneComp->GetComponentLocation();
	OutEnd = OutStart + SceneComp->GetForwardVector() * TraceLength;
}

void URushAttackSystem::AttackSphereTrace(FVector Start, FVector End, float BaseDamage, AActor* DamageCauser)
{
	FHitResult OutHit;
	TArray<AActor*> ActorsToIgnore;

	bool bHit = UKismetSystemLibrary::SphereTraceSingle(
		this,
		Start,							// 시작 위치
		End,							// 끝 위치
		TraceRadius,					// 반지름
		UEngineTypes::ConvertToTraceType(ECC_EngineTraceChannel1), // TraceChannel
		false,							// bTraceComplex
		ActorsToIgnore,					// 무시할 액터들
		DrawTraceState,					// 디버그 그리기 옵션
		OutHit,                     // Hit 결과
		true,							// Ignore Self
		FLinearColor::Red,				// Trace 색상
		FLinearColor::Green,			// Hit 색상
		TraceDrawTime					// Draw Time
	);

	if (bHit)
	{
		if (AActor* HitActor = OutHit.GetActor())
		{
			if ( auto EventManager = UDBSZEventManager::Get(GetWorld()) )
			{
				EAttackPowerType Type = AttackPowerType[ComboCount];
				EventManager->SendHitStopPair(Owner, Type, HitActor, Type);

				float DelayKnockback = 0.f;
				if (auto DataManager = UDBSZDataManager::Get(GetWorld()))
					DelayKnockback = DataManager->GetHitStopDelayTime(Type);

				FTimerDelegate TimerDelegate = FTimerDelegate::CreateLambda([this, HitActor, OutHit, Type]()
				{
					if ( !IsValid(this) )
						return;
				
					if (auto EventManager = UDBSZEventManager::Get(GetWorld()))
						EventManager->SendKnockback(HitActor, Owner, Type, 0.3f);
				});
				
				FTimerHandle TimerHandler;
				GetWorld()->GetTimerManager().SetTimer(TimerHandler, TimerDelegate, DelayKnockback, false);
			}
			
			UGameplayStatics::ApplyDamage(
				HitActor,
				BaseDamage,
				nullptr,
				DamageCauser,
				UDamageType::StaticClass()
			);
		}
	}
}

void URushAttackSystem::ResetByHit()
{
	if( Owner->IsHit )
	{
		bIsAttacking = false;
		ComboCount = 0;
	}
}

void URushAttackSystem::SetOwnerFlying()
{
	PrevMovementMode = MoveComp->MovementMode;
	MoveComp->SetMovementMode(MOVE_Flying);

	Owner->bUseControllerRotationYaw = true;
	Owner->bUseControllerRotationPitch = true;
	MoveComp->bOrientRotationToMovement = false;
}

void URushAttackSystem::PlayMontage(int32 MontageIndex)
{
	if (!AttackMontages.IsValidIndex(MontageIndex))
		return;

	bIsAttacking = true;

	AnimInstance->Montage_Play(
		AttackMontages[MontageIndex],
		1.0f,
		EMontagePlayReturnType::MontageLength,
		0.f,
		true);

	FTimerManager& TM = GetWorld()->GetTimerManager();
	TM.ClearTimer(ComboTimeHandler);
	TM.SetTimer(
		ComboTimeHandler,
		this,
		&URushAttackSystem::ResetCounter,
		ComboAttackTime,
		false
	);
}

void URushAttackSystem::DashToTarget(int32 MontageIndex)
{
    const FVector OwnerLoc = Owner->GetActorLocation();
    const FVector TargetLoc = Target->GetActorLocation();

	// XY 평면에서의 벡터와 거리를 계산
	FVector ToTargetXY = TargetLoc - OwnerLoc;
	const float DistanceXY = ToTargetXY.Size();

	// 대시 방향은 XY 평면으로, Z축은 그대로 유지
	const FVector DirXY = ToTargetXY / DistanceXY;
	const float TravelXY = FMath::Max(0.0f, DistanceXY - AttackRange);

	// 목표 지점의 Z축을 대상의 Z축으로 설정
	const FVector TargetDashWorldXY = OwnerLoc + DirXY * TravelXY;
	const FVector TargetDashWorld = FVector(TargetDashWorldXY.X, TargetDashWorldXY.Y, TargetLoc.Z);

    // Face the target (yaw only)
    {
        const FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(OwnerLoc, TargetLoc);
        FRotator NewRot(0.f, LookAt.Yaw, 0.f);
        Owner->SetActorRotation(NewRot);
    }

	// 대시 시작 위치와 목표 위치 저장
	DashStartLoc = OwnerLoc;
	DashTargetLoc = TargetDashWorld;
 
	bIsAttacking = true;
    bIsDashing = true;
	ElapsedTime = 0.0f;
    PendingMontageIndex = MontageIndex;

    PrevMovementMode = MoveComp->MovementMode;
	MoveComp->DisableMovement();

    AnimInstance->Montage_Play(DashMontages, 1.0f, EMontagePlayReturnType::MontageLength, 0.f, true);
}

void URushAttackSystem::TeleportToTarget(int32 MontageIndex)
{
	const FVector OwnerLoc = Owner->GetActorLocation();
    const FVector TargetLoc = Target->GetActorLocation();
    const FVector BackDir = Target->GetActorForwardVector() * -1.0f;
    FVector Desired = TargetLoc + BackDir * TeleportBehindOffset;

    bool bTargetAirborne = TargetMoveComp->IsFalling();
	
    const bool bGoAir = bTargetAirborne || (FMath::Abs(TargetLoc.Z - OwnerLoc.Z) > TeleportFlyZThreshold);

    if (!bGoAir && bTeleportAlignToGround)
    {
        const FVector TraceStart = Desired + FVector(0,0,500);
        const FVector TraceEnd   = Desired + FVector(0,0,-2000);
    	
        FHitResult Hit;
        TArray<AActor*> Ignore;
        bool bHit = UKismetSystemLibrary::LineTraceSingle(
            this, TraceStart, TraceEnd,
            UEngineTypes::ConvertToTraceType(ECC_Visibility), false, Ignore,
            DrawTraceState, Hit, true
        );
    	
        if (bHit)
        {
            float HalfHeight = 0.f;
            if (UCapsuleComponent* Collision = Owner->GetCapsuleComponent())
                HalfHeight = Collision->GetScaledCapsuleHalfHeight();
            Desired.Z = Hit.ImpactPoint.Z + HalfHeight;
        }
    }
    else
    {
        Desired.Z = TargetLoc.Z;
    }

    if (bGoAir)
    	SetOwnerFlying();

    Owner->SetActorLocation(Desired, false, nullptr, ETeleportType::TeleportPhysics);

    const FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(Owner->GetActorLocation(), TargetLoc);
    Owner->SetActorRotation(FRotator(0.f, LookAt.Yaw, 0.f));

	bool bNeedAirAssist =
		( (Desired.Z - OwnerLoc.Z) > TeleportFlyZThreshold) ||
		(TargetMoveComp->IsFalling()) ||
		(FMath::Abs(TargetLoc.Z - Desired.Z) > TeleportFlyZThreshold);
	
    if (bNeedAirAssist)
    	SetOwnerFlying();

    PlayMontage(MontageIndex);
}