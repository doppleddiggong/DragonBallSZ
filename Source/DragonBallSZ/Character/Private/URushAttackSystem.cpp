// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "URushAttackSystem.h"

#include "AEnemyActor.h"
#include "DragonBallSZ.h"
#include "APlayerActor.h"
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

	Owner = Cast<APlayerActor>( GetOwner() );
	MeshComp = Owner->GetMesh();
	AnimInstance = MeshComp->GetAnimInstance();

	BindMontageDelegates(AnimInstance);
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
		DashElapsedTime += DeltaTime;
        
		const float Alpha = FMath::Clamp(DashElapsedTime / DashDuration, 0.0f, 1.0f);
		const FVector Location = FMath::Lerp(DashStartLocation, DashTargetLocation, Alpha);
        
		Owner->SetActorLocation(Location, true);
        
		const float Distance = FVector::Dist2D(Owner->GetActorLocation(), DashTargetLocation);
		if ( Distance <= DashStopDistance )
		{
			PRINT_STRING(TEXT("Distance <= DashStopDistance"));
			OnRushDashCompleted();
		}
		else if ( DashElapsedTime >= DashDuration )
		{
			PRINT_STRING(TEXT("DashElapsedTime >= DashDuration"));
			OnRushDashCompleted();
		}
	}

	if ( bIsAttacking || bIsDashing )
		OnLookTarget();
}

// BeginPlay 등 한 번만 호출
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

void URushAttackSystem::OnLookTarget()
{
	if (!Owner || !Owner->TargetActor)
		return;

	const FVector TargetLoc = Owner->TargetActor->GetActorLocation();
	const FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(Owner->GetActorLocation(), TargetLoc);
	const FRotator NewRot(0.f, LookAt.Yaw, 0.f);
	Owner->SetActorRotation(NewRot);
}

void URushAttackSystem::OnAttack()
{
    if ( Owner->IsHit == false && bIsAttacking == false )
    {
        if ( bUseDashMove && !bIsDashing)
        {
            const FVector TargetLoc = Owner->TargetActor->GetActorLocation();
            const float Dist = FVector::Dist2D(Owner->GetActorLocation(), TargetLoc);
            if ( Dist > DashStopDistance )
            {
                StartRushToTarget(ComboCount);
                return;
            }
        }

        PlayAttackMontage(ComboCount);
    }
}

void URushAttackSystem::PlayAttackMontage(int32 MontageIndex)
{
	if (!MeshComp)
		return;
	
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
		3.0f,
		false
	);
}

void URushAttackSystem::OnMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& Payload)
{
	PRINT_STRING( TEXT("OnMontageNotifyBegin : %s"), *NotifyName.ToString());

	bIsAttacking = false;
	
	ComboCount++;
	if ( ComboCount > AttackMontages.Num()-1 )
		ComboCount = 0;
	
	ResetByHit();
}

void URushAttackSystem::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!bInterrupted)
	{
		if (AttackMontages.Num() > 0)
			ComboCount = (ComboCount + 1) % AttackMontages.Num();
	}
	else
	{
		ResetByHit();
	}
}

void URushAttackSystem::StartAttackTrace()
{
	GetWorld()->GetTimerManager().SetTimer(
		AttackTraceTimeHandler,
		this,
		&URushAttackSystem::AttackTrace,
		0.01f,
		false
	);
}

void URushAttackSystem::StopAttackTrace()
{
    UKismetSystemLibrary::K2_ClearAndInvalidateTimerHandle(this, AttackTraceTimeHandler);
}

void URushAttackSystem::StartRushToTarget(int32 MontageIndex)
{
    const FVector OwnerLoc = Owner->GetActorLocation();
    const FVector TargetLoc = Owner->TargetActor->GetActorLocation();

	// 1. Z축을 제외하고 XY 평면에서의 벡터와 거리를 계산
	FVector ToTargetXY = TargetLoc - OwnerLoc;
	ToTargetXY.Z = 0.0f; // Z축 무시
	const float DistanceXY = ToTargetXY.Size();

	if (DistanceXY <= DashStopDistance)
	{
		PlayAttackMontage(MontageIndex);
		return;
	}

	// 2. 대시 방향은 XY 평면으로, Z축은 그대로 유지
	const FVector DirXY = ToTargetXY / DistanceXY;
	const float TravelXY = FMath::Max(0.0f, DistanceXY - DashStopDistance);

	// 3. 목표 지점의 Z축을 대상의 Z축으로 설정
	const FVector TargetDashWorldXY = OwnerLoc + DirXY * TravelXY;
	const FVector TargetDashWorld = FVector(TargetDashWorldXY.X, TargetDashWorldXY.Y, TargetLoc.Z); // 대상의 Z축 사용

    // Face the target (yaw only)
    {
        const FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(OwnerLoc, TargetLoc);
        FRotator NewRot(0.f, LookAt.Yaw, 0.f);
        Owner->SetActorRotation(NewRot);
    }

	// 1. 대시 시작 위치와 목표 위치 저장
	DashStartLocation = OwnerLoc;
	DashTargetLocation = TargetDashWorld;
 
	bIsAttacking = true;
    bIsDashing = true;
	DashElapsedTime = 0.0f;
    PendingMontageIndex = MontageIndex;

    if (auto Movement = Owner->GetCharacterMovement())
    {
	    PrevMovementMode = Movement->MovementMode;
		Movement->DisableMovement();

    	PRINT_STRING(TEXT("Movement->DisableMovement()"));
    }

    AnimInstance->Montage_Play(DashMontages, 1.0f, EMontagePlayReturnType::MontageLength, 0.f, true);
}

void URushAttackSystem::OnRushDashCompleted()
{
    bIsDashing = false;
    AnimInstance->Montage_Stop(0.1f, DashMontages);

	if (Owner && !Owner->IsHit)
    {
		if (auto Movement = Owner->GetCharacterMovement())
        {
            Movement->SetMovementMode(PrevMovementMode);

			PRINT_STRING(TEXT("Movement->SetMovementMode(PrevMovementMode)"));
        }
    	
        PlayAttackMontage(PendingMontageIndex);
    }
}

void URushAttackSystem::AttackTrace()
{
	FVector Start, End;
	GetBodyLocation( Owner->GetBodyPart(AttackPart[ComboCount]), Start, End );
	AttackSphereTrace( Start, End, Damage, Owner);
}

void URushAttackSystem::GetBodyLocation(USceneComponent* Hand, FVector& OutStart, FVector& OutEnd) const
{
	if (!Hand)
	{
		OutStart = FVector::ZeroVector;
		OutEnd   = FVector::ZeroVector;
		return;
	}

	OutStart = Hand->GetComponentLocation();
	OutEnd = OutStart + Hand->GetForwardVector() * TraceLength;
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
			UGameplayStatics::ApplyDamage(
				HitActor,
				BaseDamage,
				nullptr,   // 필요하면 컨트롤러 전달
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