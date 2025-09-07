// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "URushAttackSystem.h"
#include "DragonBallSZ.h"
#include "APlayerActor.h"
#include "Components/ArrowComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

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

	PRINTINFO();
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

void URushAttackSystem::OnAttack()
{
	if ( Owner->IsHit == false && bIsAttacking == false )
	{
		PlayAttackByIndex(ComboCount);
	}
}

void URushAttackSystem::PlayAttackByIndex(int32 Index)
{
	if (!MeshComp)
		return;
	
	if (!AttackMontages.IsValidIndex(Index))
		return;

	bIsAttacking = true;

	AnimInstance->Montage_Play(
		AttackMontages[Index],
		AttackSpeed,
		EMontagePlayReturnType::MontageLength,
		0.f,
		true);
}

void URushAttackSystem::OnMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& Payload)
{
	PRINT_STRING( TEXT("OnMontageNotifyBegin : %s"), *NotifyName.ToString());

	bIsAttacking = false;
	
	ComboCount++;
	if ( ComboCount > 3 )
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
	// 0.01초 간격 반복 호출
	GetWorld()->GetTimerManager().SetTimer(
		AttackTimeHandler,
		this,
		&URushAttackSystem::AttackTrace,
		0.01f,
		false
	);
}

void URushAttackSystem::StopAttackTrace()
{
	UKismetSystemLibrary::K2_ClearAndInvalidateTimerHandle(this, AttackTimeHandler);
}

void URushAttackSystem::AttackTrace()
{
	if ( ComboCount == 0  || ComboCount == 2 )
	{
		// Left
		FVector Start, End;
		GetHandLocation( Owner->LeftHandComp, Start, End );
		AttackSphereTrace(Start, End, Damage, Owner);
	}
	else if ( ComboCount == 1 || ComboCount == 3)
	{
		// Right
		FVector Start, End;
		GetHandLocation( Owner->RightHandComp, Start, End );
		AttackSphereTrace(Start, End, Damage, Owner);
	}
}

void URushAttackSystem::GetHandLocation(USceneComponent* Hand, FVector& OutStart, FVector& OutEnd) const
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
		EDrawDebugTrace::ForDuration,	// 디버그 그리기 옵션
		OutHit,                     // Hit 결과
		true,							// Ignore Self
		FLinearColor::Red,				// Trace 색상
		FLinearColor::Green,			// Hit 색상
		7.0f							// Draw Time
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

void URushAttackSystem::ResetCounter()
{
	ComboCount = 0;
}

