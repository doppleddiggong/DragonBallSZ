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

	const FVector MyLoc = Owner->GetActorLocation();
	const FVector TargetLoc = Owner->TargetActor->GetActorLocation();
	const FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(MyLoc, TargetLoc);
	const FRotator NewRot(0.f, LookAt.Yaw, 0.f);
	Owner->SetActorRotation(NewRot);
}

void URushAttackSystem::OnAttack()
{
    if ( Owner->IsHit == false && bIsAttacking == false )
    {
        const bool bHasTarget = (Owner && Owner->TargetActor != nullptr);
        if (bUseDashMove && !bIsDashing && bHasTarget)
        {
            const FVector MyLoc = Owner->GetActorLocation();
            const FVector TargetLoc = Owner->TargetActor->GetActorLocation();
            const float Dist = FVector::Dist2D(MyLoc, TargetLoc);
            if ( Dist > DashStopDistance + 10.0f)
            {
                StartRushToTarget(ComboCount);
                return;
            }
        }

        OnLookTarget();
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
    if (!Owner || !Owner->TargetActor)
    {
        PlayAttackMontage(MontageIndex);
        return;
    }

    USceneComponent* RootComp = Owner->GetRootComponent();
    if (!RootComp)
    {
        PlayAttackMontage(MontageIndex);
        return;
    }

    const FVector MyLoc = Owner->GetActorLocation();
    const FVector TargetLoc = Owner->TargetActor->GetActorLocation();

    FVector ToTarget = (TargetLoc - MyLoc);
    ToTarget.Z = 0.0f;
    const float Distance = ToTarget.Size();

    if (Distance <= KINDA_SMALL_NUMBER)
    {
        PlayAttackMontage(MontageIndex);
        return;
    }

    const FVector Dir = ToTarget / Distance;
    const float Travel = FMath::Max(0.0f, Distance - DashStopDistance);
    const FVector TargetDashWorld = MyLoc + Dir * Travel;

    // Face the target (yaw only)
    {
        const FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(MyLoc, TargetLoc);
        FRotator NewRot(0.f, LookAt.Yaw, 0.f);
        Owner->SetActorRotation(NewRot);
    }

    // Prepare latent move info
    FLatentActionInfo LatentInfo;
    LatentInfo.CallbackTarget   = this;
    LatentInfo.ExecutionFunction = FName("OnRushDashCompleted");
    LatentInfo.Linkage          = 0;
    LatentInfo.UUID             = __LINE__; // reasonably unique per call site

    // Root typically has no parent; world == relative in that case
    const FVector TargetRelative = TargetDashWorld; 
    const FRotator TargetRelRot  = RootComp->GetRelativeRotation();

    bIsDashing = true;
    PendingMontageIndex = MontageIndex;

    if (UCharacterMovementComponent* Move = Owner->GetCharacterMovement())
    {
        Move->DisableMovement();
    }

    // Play dash montage while moving
    if (AnimInstance && DashMontages)
    {
        AnimInstance->Montage_Play(DashMontages, 1.0f, EMontagePlayReturnType::MontageLength, 0.f, true);
    }

    UKismetSystemLibrary::MoveComponentTo(
        RootComp,
        TargetRelative,
        TargetRelRot,
        /*bEaseOut*/ true,
        /*bEaseIn*/  true,
        /*OverTime*/ DashDuration,
        /*bForceShortestRotationPath*/ true,
        EMoveComponentAction::Type::Move,
        LatentInfo
    );
}

void URushAttackSystem::OnRushDashCompleted()
{
    bIsDashing = false;
    // Stop dash montage regardless of hit state
    if (AnimInstance && DashMontages)
    {
        AnimInstance->Montage_Stop(0.1f, DashMontages);
    }
    // If hit during dash, do not continue combo
    if (Owner && !Owner->IsHit)
    {
        if (UCharacterMovementComponent* Move = Owner->GetCharacterMovement())
        {
            Move->SetMovementMode(MOVE_Walking);
        }
    	
        // 돌진 종료 시점에 다시 한번 적을 바라보게 정렬
        OnLookTarget();
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

	// Start = 컴포넌트 월드 위치
	OutStart = Hand->GetComponentLocation();

	// 로컬 오프셋 (20,0,0)을 월드 기준으로 변환
	const FVector LocalOffset(20.f, 0.f, 0.f);
	OutEnd = Hand->GetComponentTransform().TransformPosition(LocalOffset);
}

void URushAttackSystem::AttackSphereTrace(FVector Start, FVector End, float BaseDamage, AActor* DamageCauser)
{
	FHitResult OutHit;
	TArray<AActor*> ActorsToIgnore;

	bool bHit = UKismetSystemLibrary::SphereTraceSingle(
		this,
		Start,							// 시작 위치
		End,							// 끝 위치
		20.0f,							// 반지름
		UEngineTypes::ConvertToTraceType(ECC_Visibility), // TraceChannel
		false,							// bTraceComplex
		ActorsToIgnore,					// 무시할 액터들
		DrawTraceState,					// 디버그 그리기 옵션
		OutHit,                     // Hit 결과
		true,							// Ignore Self
		FLinearColor::Red,				// Trace 색상
		FLinearColor::Green,			// Hit 색상
		3.0f							// Draw Time
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