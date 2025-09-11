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

    // 공격 중 자동 추적(간단 2D 기반, 루트모션 시 비활성)
	if ( bIsAttacking && !bIsDashing && Owner && Target )
	{
		const FVector OwnerLoc = Owner->GetActorLocation();
		const FVector ToTarget2D = (Target->GetActorLocation() - OwnerLoc).GetSafeNormal2D();
		// 회전 보정(Yaw만)
		const float DesiredYaw = ToTarget2D.Rotation().Yaw;
		const FRotator NewRot = FMath::RInterpConstantTo(Owner->GetActorRotation(), FRotator(0.f, DesiredYaw, 0.f), DeltaTime, AutoTrackTurnRateDeg);
		Owner->SetActorRotation(FRotator(0.f, NewRot.Yaw, 0.f));

		if (AutoTrackMoveSpeed > 0.f)
		{
			const FVector NewLoc = OwnerLoc + Owner->GetActorForwardVector() * (AutoTrackMoveSpeed * DeltaTime);
			Owner->SetActorLocation(NewLoc, true);
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

	MoveComp->DisableMovement();

	EventManager->SendDash(Owner, true);
    AnimInstance->Montage_Play(DashMontages, 1.0f, EMontagePlayReturnType::MontageLength, 0.f, true);
}

void URushAttackSystem::TeleportToTarget(int32 MontageIndex)
{
    const FVector TargetLoc = Target->GetActorLocation();

    // 1) 기본 후보: 타겟이 '서 있다고 가정'한 Yaw 기준의 뒤편(Forward의 Pitch/Roll 무시)
    const float DesiredRadius = FMath::Clamp(TeleportBehindOffset, 50.f, AttackRange * 0.9f);
    const float TargetYaw = Target->GetActorRotation().Yaw; // Pitch/Roll 무시
    const FVector UprightForward = FRotator(0.f, TargetYaw, 0.f).Vector();
    FVector Desired = TargetLoc - UprightForward * DesiredRadius;

    // 2) 지면 정렬/공중 고려로 Z 보정 함수
    auto AlignZ = [&](const FVector& In) -> FVector
    {
        FVector Out = In;
        if (bTeleportAlignToGround)
        {
            FHitResult Hit;
            const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, In + FVector(0,0,500), In - FVector(0,0,2000), ECC_Visibility);
            if (bHit)
            {
                const float HalfHeight = Owner->GetCapsuleComponent() ? Owner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 0.f;
                Out.Z = Hit.ImpactPoint.Z + HalfHeight;
                return Out;
            }
        }
        // 지면 정렬 실패 시: 타겟 Z 유지
        Out.Z = TargetLoc.Z;
        return Out;
    };

    // 3) 위치 충돌 검사: 캡슐 오버랩로 블로킹 여부 확인
    auto IsFree = [&](const FVector& Pos) -> bool
    {
        const UCapsuleComponent* Cap = Owner->GetCapsuleComponent();
        const float R = Cap ? Cap->GetScaledCapsuleRadius() : 34.f;
        const float H = Cap ? Cap->GetScaledCapsuleHalfHeight() : 88.f;
        FCollisionQueryParams QP(SCENE_QUERY_STAT(TeleportProbe), false, Owner);
        return !GetWorld()->OverlapBlockingTestByChannel(Pos, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeCapsule(R, H), QP);
    };

    // 4) 후보 탐색: 뒤쪽 우선, 좌우로 각도 샘플링하며 AttackRange 원주상에서 빈자리 찾기
    const int32 Samples = 10;
    const float BaseYaw = TargetYaw + 180.f; // 뒤편 기준
    FVector Chosen = AlignZ(Desired);
    if (!IsFree(Chosen))
    {
        bool bFound = false;
        for (int32 i=1; i<=Samples; ++i)
        {
            const float Angle = 20.f * i; // 20deg step
            for (int s=-1; s<=1; s+=2)
            {
                const float Yaw = BaseYaw + s*Angle;
                const FVector Dir = FRotationMatrix(FRotator(0, Yaw, 0)).GetUnitAxis(EAxis::X);
                const FVector Cand = TargetLoc + Dir * DesiredRadius;
                const FVector CandZ = AlignZ(Cand);
                if (IsFree(CandZ))
                {
	                Chosen = CandZ;
                	bFound = true;
                	break;
                }
            }

        	if (bFound)
            	break;
        }
        // 최종 실패 시: 타겟 주변으로 강제 스윕 텔레포트(충돌 무시)
    }

    // 5) 텔레포트 및 회전: 타겟을 바라보게 정렬
    Owner->SetActorLocation(Chosen, false, nullptr, ETeleportType::TeleportPhysics);
    const FRotator Face = (TargetLoc - Chosen).Rotation();
    Owner->SetActorRotation(FRotator(0.f, Face.Yaw, 0.f));
    EventManager->SendTeleport(Owner);

    // 6) 공중 콤보 보조: 타겟이 공중이면 비행 모드로
    {
        FHitResult TargetTraceHit;
        const bool bTargetOnGround = GetWorld()->LineTraceSingleByChannel(TargetTraceHit, TargetLoc, TargetLoc - FVector(0,0,1000.f), ECC_Visibility);
        if (!bTargetOnGround)
        {
        	Owner->SetFlying();
        }
    }

    PlayMontage(MontageIndex);
}
