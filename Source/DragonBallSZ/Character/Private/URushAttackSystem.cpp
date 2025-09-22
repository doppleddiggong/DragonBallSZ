// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "URushAttackSystem.h"

#include "UDBSZEventManager.h"
#include "UDBSZDataManager.h"
#include "TimerManager.h"
#include "ACombatCharacter.h"
#include "UCharacterData.h"

#include "DragonBallSZ.h"
#include "UDBSZFunctionLibrary.h"

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

	/*
	 * bIsDashing 상태 제어 및 대시 이동 처리 로직
	 *
	 * 1. 시작: OnAttack() -> DashToTarget() 함수가 호출되면서 bIsDashing = true로 설정됩니다.
	 *    - 이 때, 대시 시작/목표 위치(DashStartLoc/DashTargetLoc)와 대시 시간(DashDuration)이 계산됩니다.
	 *
	 * 2. 진행 (매 틱): bIsDashing이 true인 동안 아래 로직이 실행됩니다.
	 *    - 경과 시간(ElapsedTime)을 계속 누적합니다.
	 *    - 경과 시간과 총 대시 시간(DashDuration)을 이용해 보간 알파값(Alpha)을 계산합니다.
	 *    - Lerp를 사용해 시작 위치와 목표 위치 사이를 부드럽게 이동시킵니다.
	 *
	 * 3. 종료: 아래 두 조건 중 하나라도 만족하면 OnDashCompleted()가 호출됩니다.
	 *    - bArrived: 캐릭터가 목표 지점에 도착했을 때
	 *    - bDurationExpired: 대시 시간이 모두 경과했을 때
	 *
	 * 4. 완료: OnDashCompleted() 함수 내에서 bIsDashing = false로 설정되어, 다음 틱부터 이 블록은 실행되지 않습니다.
	 *    - 이후 대시 몽타주를 중지하고, 대기 중이던 공격 몽타주(PendingMontageIndex)를 재생합니다.
	 */
	if (bIsDashing)
	{
        if (!IsValid(Owner)) // Add this check
        {
            PRINTLOG(TEXT("TickComponent: Owner is invalid while dashing. Stopping dash."));
            OnDashCompleted(); // Stop dashing if owner is invalid
            return;
        }
		ElapsedTime += DeltaTime;
		
		const float SafeDashDuration = FMath::Max(DashDuration, KINDA_SMALL_NUMBER);
		const float Alpha = FMath::Clamp(ElapsedTime / SafeDashDuration, 0.0f, 1.0f);
		const FVector Location = FMath::Lerp(DashStartLoc, DashTargetLoc, Alpha);
		
		Owner->SetActorLocation(Location, true);
		
		const FVector CurrentLoc = Owner->GetActorLocation();
		const float DistanceSq = FVector::DistSquared(CurrentLoc, DashTargetLoc);
		const float ArrivalToleranceSq = FMath::Square(FMath::Max(DashArrivalTolerance, 0.0f));

        const bool bArrived = DistanceSq <= ArrivalToleranceSq;
        const bool bDurationExpired = ElapsedTime >= SafeDashDuration;
        if ( bArrived || bDurationExpired )		
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

			FVector OwnerProjectionLoc = FVector(Owner->GetActorLocation().X, Owner->GetActorLocation().Y, 0.f);
			FVector TargetProjectionLoc = FVector(Target->GetActorLocation().X, Target->GetActorLocation().Y, 0.f);
			float ProjectionDist = FVector::Dist(OwnerProjectionLoc, TargetProjectionLoc);

			if (ProjectionDist > 90) Owner->SetActorLocation(NewLoc, true);
		}
	}

	if ( ComboResetTime > 0.0f &&
		GetWorld()->GetTimeSeconds() > ComboResetTime )
	{
		ResetComboCount();
	}
}

void URushAttackSystem::BindMontageDelegates(UAnimInstance* Anim)
{
	if (!Anim || bDelegatesBound)
		return;

	// 중복 방지용으로 먼저 제거
	Anim->OnMontageEnded          .RemoveDynamic(this, &URushAttackSystem::OnMontageEnded);
	Anim->OnMontageEnded          .AddDynamic(this, &URushAttackSystem::OnMontageEnded);

	bDelegatesBound = true;
}

void URushAttackSystem::UnbindMontageDelegates(UAnimInstance* Anim)
{
	if (!Anim || !bDelegatesBound)
		return;

	Anim->OnMontageEnded          .RemoveDynamic(this, &URushAttackSystem::OnMontageEnded);

	bDelegatesBound = false;
}

void URushAttackSystem::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (bInterrupted && Owner->IsHitting() )
	{
		bIsAttacking = false;
		ComboCount = 0;
	}

    // 몽타주가 정상적으로 끝났을 때 (중단되지 않았을 때)
    if (!bInterrupted)
    {
        bIsAttacking = false;
        ComboCount = 0;
        // Only recover movement mode if the owner is not currently flying
        if (Owner && Owner->GetCharacterMovement()->MovementMode != MOVE_Flying)
        {
            Owner->RecoveryMovementMode(PrevMovementMode);
        }
    }
}

void URushAttackSystem::InitSystem(ACombatCharacter* InOwner, UCharacterData* InData)
{
	this->Owner = InOwner;

	MeshComp = Owner->GetMesh();
	AnimInstance = MeshComp->GetAnimInstance();
	MoveComp = Owner->GetCharacterMovement();

	this->Target = Owner->TargetActor;
	if (IsValid(Target))
	{
		TargetMoveComp = Target->GetCharacterMovement();
	}
	else
	{
		PRINTLOG(TEXT("InitSystem: Target is not valid. Owner: %s"), *Owner->GetName());
	}

	EventManager = UDBSZEventManager::Get(GetWorld());

	if (IsValid(InData))
	{
		InData->LoadRushMontage(AttackMontages);
		InData->LoadRushPower(AttackPowerType);
		InData->LoadDashMontage(DashMontage);
		if (!IsValid(DashMontage))
		{
			PRINTLOG(TEXT("InitSystem: DashMontage failed to load from InData."));
		}
	}
	else
	{
		PRINTLOG( TEXT("InitSystem: InData is not valid."));
	}
	
	BindMontageDelegates(AnimInstance);
}

void URushAttackSystem::OnDashCompleted()
{
	bIsDashing = false;
    if (IsValid(AnimInstance) && IsValid(DashMontage)) // Add check for DashMontage
    {
	    AnimInstance->Montage_Stop(0.1f, DashMontage);
    }

	if (!Owner->IsHitting())
		PlayMontage();

	Owner->RecoveryMovementMode(PrevMovementMode);
	EventManager->SendDash(Owner, false, FVector::ZeroVector );
}

void URushAttackSystem::OnAttack()
{
    if (!IsValid(Target))
    {
        PRINTLOG(TEXT("OnAttack: Target is invalid. Skipping attack."));
        return;
    }
	
	if (Owner->IsAttackEnable() == false )
        return;

	if ( bIsAttacking )
		return;

	const FVector OwnerLoc = Owner->GetActorLocation();
	const FVector TargetLoc = Target->GetActorLocation();
	const float Dist = FVector::Dist(OwnerLoc, TargetLoc);

	if (Dist >= TeleportRange)
	{
		TeleportToTarget();
	}
	else if (!bIsDashing)
	{
		DashToTarget();
	}
	else
	{
		PlayMontage();
	}
}

void URushAttackSystem::StartAttackTrace()
{
	ComboCount++;
	if ( ComboCount > AttackPowerType.Num()-1 )
		ComboCount = 0;

	PRINT_STRING(TEXT("ComboCount : %d"), ComboCount);
	
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


EAttackPowerType URushAttackSystem::GetAttackPower(int InCount)
{
	if (AttackPowerType.IsValidIndex(InCount))
		return AttackPowerType[InCount];

	return EAttackPowerType::Small;
}

void URushAttackSystem::AttackTrace()
{
	if ( !Owner->IsInSight( Target ))
		return;

	const EAttackPowerType Type = GetAttackPower(ComboCount);
	float DelayKnockback = 0.f;
	if (auto DataManager = UDBSZDataManager::Get(GetWorld()))
		DelayKnockback = DataManager->GetHitStopDelayTime(Type);

	EventManager->SendHitStopPair(Owner, Type, Target, Type);

	FTimerDelegate TimerDelegate = FTimerDelegate::CreateWeakLambda(this, [this, Type]()
	{
		EventManager->SendKnockback(Target, this->Owner, Type, 0.3f);
	});

	Owner->SetAttackChargeKi(ComboCount);
	Owner->PlaySoundAttack();
	
	GetWorld()->GetTimerManager().SetTimer(KnockbackTimerHandler, TimerDelegate, DelayKnockback, false);
	UGameplayStatics::ApplyDamage(
		Target,
		Owner->GetAttackDamage(ComboCount),
		nullptr,
		Owner,
		UDBSZFunctionLibrary::GetDamageTypeClass(Type)
	);
}

void URushAttackSystem::PlayMontage()
{
	// 시스템이 준비되지 않았거나 월드가 유효하지 않으면 즉시 중단
    if (!Owner || !GetWorld() || !EventManager || !AnimInstance)
    {
        PRINTLOG(TEXT("PlayMontage가 시스템 준비 전에 호출되어 중단됩니다."));
        return;
    }

	bIsAttacking = true;

	LastAttackTime = GetWorld()->GetTimeSeconds();
	
	if (IsValid(AnimInstance) && IsValid(AttackMontages) )
    {
		float AttackEndTime = AttackMontages->GetPlayLength();
		ComboResetTime = GetWorld()->GetTimeSeconds() + AttackEndTime + ComboResetTime_Offset;

		const FVector TargetLoc = Target->GetActorLocation();
		FHitResult TargetTraceHit;
		const bool bTargetOnGround = GetWorld()->LineTraceSingleByChannel(TargetTraceHit, TargetLoc, TargetLoc - FVector(0,0,1000.f), ECC_Visibility);
		if (!bTargetOnGround)
			Owner->SetFlying();
	
	    AnimInstance->Montage_Play(
		    AttackMontages,
		    1.0f,
		    EMontagePlayReturnType::MontageLength,
		    0.f,
		    true);
    }
}

void URushAttackSystem::DashToTarget()
{
    if (!IsValid(Target))
    {
        PRINTLOG(TEXT("DashToTarget: Target is invalid. Skipping dash."));
        PlayMontage();
        return;
    }
	
    const FVector OwnerLoc = Owner->GetActorLocation();
    const FVector TargetLoc = Target->GetActorLocation();

	// XY 평면에서의 벡터와 거리를 계산
	FVector ToTargetXY = TargetLoc - OwnerLoc;
	const float DistanceXY = ToTargetXY.Size();

	if ( DistanceXY < KINDA_SMALL_NUMBER )
	{
		PRINTLOG(TEXT("DashToTarget: DistanceXY is too small, skipping dash. Owner: %s, Target: %s"), *Owner->GetName(), *Target->GetName());
		PlayMontage();
		return;
	}

	// 대시 방향은 XY 평면으로, Z축은 그대로 유지
	const FVector DirXY = ToTargetXY / DistanceXY;
	const float TravelXY = FMath::Max(0.0f, DistanceXY - AttackRange);

	// 목표 지점의 Z축을 대상의 Z축으로 설정
	const FVector TargetDashWorldXY = OwnerLoc + DirXY * TravelXY;
	const FVector TargetDashWorld = FVector(TargetDashWorldXY.X, TargetDashWorldXY.Y, TargetLoc.Z);

	// 대시 시작 위치와 목표 위치 저장
	DashStartLoc = OwnerLoc;
	DashTargetLoc = TargetDashWorld;

	float Distance = FVector::Dist(DashStartLoc, DashTargetLoc);
	// 거리 기준 최대/최소값 설정 (원하는 값으로 조정)
	float MinDistance = 100.f;
	float MaxDistance = 2000.f;
	// 거리 비례해서 DashDuration 계산 (0.05 ~ 1 사이)
	DashDuration = FMath::GetMappedRangeValueClamped(
		FVector2D(MinDistance, MaxDistance),
		FVector2D(0.05f, 1.f),
		Distance
	);	
	
	bIsAttacking = true;
    bIsDashing = true;
	
	ElapsedTime = 0.0f;

	PrevMovementMode = MoveComp->MovementMode;
	MoveComp->DisableMovement();


	if ( TravelXY > DashEventRange )
	{
		EventManager->SendDash(Owner, true, (DashTargetLoc - DashStartLoc) );
	}
	
    if (IsValid(AnimInstance) && IsValid(DashMontage) )
    {
		AnimInstance->Montage_Play(DashMontage, 1.0f, EMontagePlayReturnType::MontageLength, 0.f, true);
    }
}

void URushAttackSystem::TeleportToTarget()
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
	Owner->PlaySoundTeleport();
	
    Owner->SetActorLocation(Chosen, false, nullptr, ETeleportType::TeleportPhysics);
    const FRotator Face = (TargetLoc - Chosen).Rotation();
    Owner->SetActorRotation(FRotator(0.f, Face.Yaw, 0.f));
    EventManager->SendTeleport(Owner);

    // // 6) 공중 콤보 보조: 타겟이 공중이면 비행 모드로
    // {
    //     FHitResult TargetTraceHit;
    //     const bool bTargetOnGround = GetWorld()->LineTraceSingleByChannel(TargetTraceHit, TargetLoc, TargetLoc - FVector(0,0,1000.f), ECC_Visibility);
    //     if (!bTargetOnGround)
    //     {
    //     	Owner->SetFlying();
    //     }
    // }

	PrevMovementMode = MoveComp->MovementMode;
	PlayMontage();
}