// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "URushAttackSystem.h"

#include "AEnemyActor.h"
#include "APlayerActor.h"

#include "UDBSZEventManager.h"
#include "UDBSZDataManager.h"
#include "TimerManager.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

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

    // 공격 중 자동 추적
	if ( bEnableAutoTrackDuringAttack )
	{
		if ( bIsAttacking && !bIsDashing )
		{
			const FVector ToTarget = (Target->GetActorLocation() - Owner->GetActorLocation());
			const FRotator CurRot = Owner->GetActorRotation();
			const FRotator DesiredRot = ToTarget.Rotation();
			const FRotator NewRot = FMath::RInterpConstantTo(CurRot, DesiredRot, DeltaTime, AutoTrackTurnRateDeg);

			Owner->SetActorRotation(FRotator(0.f, NewRot.Yaw, 0.f));
			const FVector MoveDir = Owner->GetActorForwardVector();

			if (AutoTrackMoveSpeed > 0.f)
			{
				const FVector NewLoc = Owner->GetActorLocation() + MoveDir.GetSafeNormal() * (AutoTrackMoveSpeed * DeltaTime);
				Owner->SetActorLocation(NewLoc, true);
			}
		}
	}
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
}

void URushAttackSystem::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (bInterrupted && Owner->IsHiting() )
	{
		bIsAttacking = false;
		ComboCount = 0;
	}
}

void URushAttackSystem::InitSystem(APlayerActor* InOwner)
{
	this->Owner = InOwner;

	MeshComp = Owner->GetMesh();
	AnimInstance = MeshComp->GetAnimInstance();
	MoveComp = Owner->GetCharacterMovement();

	this->Target = Owner->TargetActor;
	TargetMoveComp = Target->GetCharacterMovement();

	EventManager = UDBSZEventManager::Get(GetWorld());

	BindMontageDelegates(AnimInstance);
}

void URushAttackSystem::OnDashCompleted()
{
	bIsDashing = false;
	AnimInstance->Montage_Stop(0.1f, DashMontages);

	if (Owner && !Owner->IsHit)
	{
		EventManager->SendDash(Owner, false);
		
		MoveComp->SetMovementMode(PrevMovementMode);
		PlayMontage(PendingMontageIndex);
	}
}

void URushAttackSystem::OnAttack()
{
	if (Owner->IsAttackEnable() == false )
        return;

	if ( bIsAttacking )
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
	if ( !Owner->IsInSight( Target ))
		return;

	const EAttackPowerType Type = AttackPowerType[ComboCount];
	float DelayKnockback = 0.f;
	if (auto DataManager = UDBSZDataManager::Get(GetWorld()))
		DelayKnockback = DataManager->GetHitStopDelayTime(Type);

	EventManager->SendHitStopPair(Owner, Type, Target, Type);

	FTimerDelegate TimerDelegate = FTimerDelegate::CreateWeakLambda(this, [this, Type]()
	{
		EventManager->SendKnockback(Target, this->Owner, Type, 0.3f);
	});

	GetWorld()->GetTimerManager().SetTimer(KnockbackTimerHandler, TimerDelegate, DelayKnockback, false);

	UGameplayStatics::ApplyDamage(
		Target,
		Damage,
		nullptr,
		Owner,
		UDamageType::StaticClass()
	);
}

// void URushAttackSystem::GetBodyLocation(USceneComponent* SceneComp, FVector& OutStart, FVector& OutEnd) const
// {
// 	if (!SceneComp)
// 	{
// 		OutStart = FVector::ZeroVector;
// 		OutEnd   = FVector::ZeroVector;
// 		return;
// 	}
//
// 	OutStart = SceneComp->GetComponentLocation();
// 	OutEnd = OutStart + SceneComp->GetForwardVector() * TraceLength;
// }

// void URushAttackSystem::AttackSphereTrace(FVector Start, FVector End, float BaseDamage, AActor* DamageCauser)
// {
// 	FHitResult OutHit;
// 	TArray<AActor*> ActorsToIgnore;
//
// 	bool bHit = UKismetSystemLibrary::SphereTraceSingle(
// 		this,
// 		Start,							// 시작 위치
// 		End,							// 끝 위치
// 		TraceRadius,					// 반지름
// 		UEngineTypes::ConvertToTraceType(ECC_EngineTraceChannel1), // TraceChannel
// 		false,							// bTraceComplex
// 		ActorsToIgnore,					// 무시할 액터들
// 		DrawTraceState,					// 디버그 그리기 옵션
// 		OutHit,                     // Hit 결과
// 		true,							// Ignore Self
// 		FLinearColor::Red,				// Trace 색상
// 		FLinearColor::Green,			// Hit 색상
// 		TraceDrawTime					// Draw Time
// 	);
//
// 	if (bHit)
// 	{
// 		if (AActor* HitActor = OutHit.GetActor())
// 		{
// 			EAttackPowerType Type = AttackPowerType[ComboCount];
// 			EventManager->SendHitStopPair(
// 				Owner, Type,
// 				HitActor, Type);
//
// 			float DelayKnockback = 0.f;
// 			if (auto DataManager = UDBSZDataManager::Get(GetWorld()))
// 				DelayKnockback = DataManager->GetHitStopDelayTime(Type);
//
// 			FTimerDelegate TimerDelegate = FTimerDelegate::CreateLambda([this, HitActor, Type]()
// 			{
// 				if ( !IsValid(this))
// 					return;
//
// 				EventManager->SendKnockback(HitActor, this->Owner, Type, 0.3f);
// 			});
// 			
// 			FTimerHandle TimerHandler;
// 			GetWorld()->GetTimerManager().SetTimer(TimerHandler, TimerDelegate, DelayKnockback, false);
// 			
// 			UGameplayStatics::ApplyDamage(
// 				HitActor,
// 				BaseDamage,
// 				nullptr,
// 				DamageCauser,
// 				UDamageType::StaticClass()
// 			);
// 		}
// 	}
// }


// EMovementMode URushAttackSystem::SetOwnerFlying()
// {
// 	PrevMovementMode = MoveComp->MovementMode;
// 	MoveComp->SetMovementMode(MOVE_Flying);
//
// 	Owner->bUseControllerRotationYaw = true;
// 	Owner->bUseControllerRotationPitch = true;
// 	MoveComp->bOrientRotationToMovement = false;
// }

void URushAttackSystem::PlayMontage(int32 MontageIndex)
{
	if (!AttackMontages.IsValidIndex(MontageIndex))
		return;

	bIsAttacking = true;

	EventManager->SendAttack(Owner, MontageIndex);
	
	AnimInstance->Montage_Play(
		AttackMontages[MontageIndex],
		1.0f,
		EMontagePlayReturnType::MontageLength,
		0.f,
		true);
	
	GetWorld()->GetTimerManager().ClearTimer(ComboTimeHandler);
	GetWorld()->GetTimerManager().SetTimer(
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

	// 대시 시작 위치와 목표 위치 저장
	DashStartLoc = OwnerLoc;
	DashTargetLoc = TargetDashWorld;
 
	bIsAttacking = true;
    bIsDashing = true;
	
	ElapsedTime = 0.0f;
    PendingMontageIndex = MontageIndex;

    PrevMovementMode = MoveComp->MovementMode;
	MoveComp->DisableMovement();

	EventManager->SendDash(Owner, true);
    AnimInstance->Montage_Play(DashMontages, 1.0f, EMontagePlayReturnType::MontageLength, 0.f, true);
}

void URushAttackSystem::TeleportToTarget(int32 MontageIndex)
{
	// 1. 목표물 위치와 플레이어 위치를 기반으로 Desired 위치 계산 (Z축은 제외)
    const FVector TargetLoc = Target->GetActorLocation();
    const FVector BackDir = Target->GetActorForwardVector() * -1.0f;
    FVector TeleportLoc = TargetLoc + BackDir * TeleportBehindOffset;

	// 2. 목표물이 공중에 있는지 확인 (단 한번만)
	FHitResult TargetTraceHit;
	bool bTargetOnGround = GetWorld()->LineTraceSingleByChannel(
		TargetTraceHit, 
		TargetLoc, 
		TargetLoc - FVector(0, 0, 1000.f), 
		ECC_Visibility
	);

	// 3. 목표물 상태에 따라 Desired Z값 결정
	if (bTargetOnGround)
	{
		// 지상에 있을 경우, 목표물 높이로 설정
		TeleportLoc.Z = TargetLoc.Z;
	}
	else
	{
		// 공중에 있을 경우
		// 지면 정렬 모드라면, 텔레포트 지점의 지면을 트레이스
		FHitResult PlayerTraceHit;
		bool bPlayerGroundFound = GetWorld()->LineTraceSingleByChannel(
			PlayerTraceHit, 
			TeleportLoc + FVector(0, 0, 500), 
			TeleportLoc - FVector(0, 0, 2000), 
			ECC_Visibility
		);

		if (bPlayerGroundFound)
		{
			// 지면을 찾았을 경우 Z값 조정
			float HalfHeight = Owner->GetCapsuleComponent() ? Owner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 0.f;
			TeleportLoc.Z = PlayerTraceHit.ImpactPoint.Z + HalfHeight;
		}
		else
		{
			// 지면을 찾지 못하면 목표물 높이로 설정 (공중 콤보로 이어짐)
			TeleportLoc.Z = TargetLoc.Z;
		}
	}

	// 4. 최종적으로 캐릭터 텔레포트 및 애니메이션 재생
	Owner->SetActorLocation(TeleportLoc, false, nullptr, ETeleportType::TeleportPhysics);
	EventManager->SendTeleport(Owner);
	
	// 5. 공중 콤보 보조 로직 (별도로 분리 가능)
	if (!bTargetOnGround)
	{
		PrevMovementMode = Owner->SetFlying();
	}
	
    PlayMontage(MontageIndex);
}