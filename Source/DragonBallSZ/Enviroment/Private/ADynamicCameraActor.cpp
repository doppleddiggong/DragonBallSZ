// Fill out your copyright notice in the Description page of Project Settings.


#include "ADynamicCameraActor.h"

#include "AEnemyActor.h"
#include "APlayerActor.h"

#include "DragonBallSZ.h"
#include "UDBSZEventManager.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"


// Sets default values
ADynamicCameraActor::ADynamicCameraActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SetRootComponent(SpringArmComp);
	
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);
}

// Called when the game starts or when spawned
void ADynamicCameraActor::BeginPlay()
{
	Super::BeginPlay();

	PlayerRef = Cast<APlayerActor>(UGameplayStatics::GetActorOfClass(GetWorld(), APlayerActor::StaticClass()));
	TargetRef = Cast<AEnemyActor>(UGameplayStatics::GetActorOfClass(GetWorld(), AEnemyActor::StaticClass()));

	// 이벤트 매니저를 통한 이벤트 등록및 제어
	EventManager = UDBSZEventManager::Get(GetWorld());
	EventManager->OnDash.AddDynamic(this, &ADynamicCameraActor::OnDash);
	EventManager->OnTeleport.AddDynamic(this, &ADynamicCameraActor::OnTeleport);
	EventManager->OnAttack.AddDynamic(this, &ADynamicCameraActor::OnAttack);
	EventManager->OnSpecialAttack.AddDynamic(this, &ADynamicCameraActor::OnSpecialAttack);
	EventManager->OnGuard.AddDynamic(this, &ADynamicCameraActor::OnGuard);
	EventManager->OnAvoid.AddDynamic(this, &ADynamicCameraActor::OnAvoid);
	EventManager->OnPowerCharge.AddDynamic(this, &ADynamicCameraActor::OnPowerCharge);

	
}

// Called every frame
void ADynamicCameraActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!PlayerRef || !TargetRef)
	{
		return;
	}

	CurrentDistance = FVector::Dist(PlayerRef->GetActorLocation(), TargetRef->GetActorLocation());

	PlayerRotationLock();

	// 근접 상태일 때는 이전과 동일하게 플레이어 등 뒤를 따라갑니다.
	if (CurrentDistance < TargetDistance)
	{
		ResetCameraLocation(DeltaTime);
		CloseCameraRotation(DeltaTime);
	}
	// 원거리 상태일 때의 로직
	else
	{
		// ✅ 이 프레임에서 카메라를 리셋해야 하는지 결정할 변수
		bool bShouldResetCameraNow = false;

		// 조건 1: 플레이어가 카메라-타겟 직선에서 너무 멀리 벗어났는가?
		if (ShouldResetByAlignment())
		{
			bShouldResetCameraNow = true;
		}

		// 조건 2: (위 조건에 해당하지 않을 때) 플레이어가 앞 또는 뒤로 움직이는가?
		if (!bShouldResetCameraNow)
		{
			const FVector PlayerVelocity = PlayerRef->GetVelocity();
			if (!PlayerVelocity.IsNearlyZero())
			{
				const FVector DirectionToTarget = (TargetRef->GetActorLocation() - PlayerRef->GetActorLocation()).GetSafeNormal();
				const float ForwardDot = FVector::DotProduct(PlayerVelocity.GetSafeNormal(), DirectionToTarget);
				
				// 전후방 움직임 감지
				if (FMath::Abs(ForwardDot) > 0.5f)
				{
					bShouldResetCameraNow = true;
				}
			}
		}
		
		// ✅ 위 조건 중 하나라도 만족하면 카메라를 움직입니다.
		if (bShouldResetCameraNow)
		{
			// 카메라를 플레이어 등 뒤로 부드럽게 이동시킵니다.
			ResetCameraLocation(DeltaTime);
			CloseCameraRotation(DeltaTime);
		}
		// 모든 조건에 해당하지 않으면(예: 얼라인먼트 안에서 옆으로 움직일 때)
		// 카메라는 완벽하게 고정됩니다.
	}
}

void ADynamicCameraActor::PlayerRotationLock()
{
	FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(PlayerRef->GetActorLocation(), TargetRef->GetActorLocation());
	FRotator LockRot = FRotator(PlayerRef->GetActorRotation().Pitch, TargetRot.Yaw,PlayerRef->GetActorRotation().Roll);
	PlayerRef->SetActorRotation(LockRot);
}

void ADynamicCameraActor::CloseCameraRotation(float DeltaTime)
{
	FRotator NewRot = UKismetMathLibrary::RInterpTo(GetActorRotation(), UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargetRef->GetActorLocation()), DeltaTime, 5);
	SetActorRotation(NewRot);
}

void ADynamicCameraActor::ResetCameraLocation(float DeltaTime)
{
	// ✅ 타겟 가림 회피가 적용된 최종 목표 위치를 가져옵니다.
	FVector TargetCameraLocation = GetAvoidanceAdjustedCameraLocation();
	
	// 목표 위치로 부드럽게 이동합니다.
	FVector NewLocation = UKismetMathLibrary::VInterpTo(GetActorLocation(), TargetCameraLocation, DeltaTime, 5.f);
	SetActorLocation(NewLocation);
}

void ADynamicCameraActor::ResetCameraForwardLocation(float DeltaTime)
{
	FVector PlayerLocation = PlayerRef->GetActorLocation();
	FVector TargetLocation = TargetRef->GetActorLocation();
	FVector NewLocation = UKismetMathLibrary::VInterpTo(GetActorLocation(), ((PlayerLocation + (PlayerLocation - TargetLocation).GetSafeNormal() * 300).Length() * GetActorForwardVector()* -1) + FVector(0, 0, 100), DeltaTime, 1);
	SetActorLocation(NewLocation);
}

void ADynamicCameraActor::ResetCameraRotation(float DeltaTime)
{
	FVector PlayerLocation = PlayerRef->GetActorLocation();
	FVector TargetLocation = TargetRef->GetActorLocation();
	FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), (PlayerLocation + TargetLocation) / 2);
	FRotator NewRotation = UKismetMathLibrary::RInterpTo(GetActorRotation(), TargetRot, DeltaTime, 5);
	SetActorRotation(NewRotation);
}

bool ADynamicCameraActor::TargetDeadZoneCheck(const AActor& Target)
{
	const APlayerActor * TargetIsPlayer = Cast<APlayerActor>(&Target);

	float MinX, MaxX, MinY, MaxY;
	
	if (TargetIsPlayer)
	{
		MinX = DeadZonePlayer_X_Min;
		MaxX = DeadZonePlayer_X_Max;
		MinY = DeadZonePlayer_Y_Min;
		MaxY = DeadZonePlayer_Y_Max;
	}
	else
	{
		MinX = DeadZoneTarget_X_Min;
		MaxX = DeadZoneTarget_X_Max;
		MinY = DeadZoneTarget_Y_Min;
		MaxY = DeadZoneTarget_Y_Max;
	}
	
	FVector2D ScreenPos;
	if (!UGameplayStatics::ProjectWorldToScreen(GetWorld()->GetFirstPlayerController(), Target.GetActorLocation(), ScreenPos, true))
	{
		return true;
	}
	
	FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(GetWorld());
	
	// 뷰포트 크기가 0이 되어 나누기 오류가 발생하는 것을 방지합니다.
	if (ViewportSize.X < 1.f || ViewportSize.Y < 1.f)
	{
		return false;
	}
	
	ScreenPos.X /= ViewportSize.X;
	ScreenPos.Y /= ViewportSize.Y;

	// 캐릭터가 데드존 '밖'에 있는지 확인합니다.
	if (ScreenPos.X < MinX || ScreenPos.X > MaxX || ScreenPos.Y < MinY || ScreenPos.Y > MaxY)
	{
		return true; // 데드존 밖에 있으므로 true를 반환
	}

	return false; // 데드존 안에 있으므로 false를 반환
}

bool ADynamicCameraActor::ShouldResetByAlignment() const
{
	// 계산에 필요한 세 지점의 위치를 가져옵니다.
	const FVector CameraLocation = GetActorLocation();
	const FVector PlayerLocation = PlayerRef->GetActorLocation();
	const FVector TargetLocation = TargetRef->GetActorLocation();

	// 카메라->타겟 벡터와 카메라->플레이어 벡터를 계산합니다.
	const FVector CamToTarget = TargetLocation - CameraLocation;
	const FVector CamToPlayer = PlayerLocation - CameraLocation;

	// 두 벡터의 외적(Cross Product)을 이용해 플레이어와 직선 사이의 거리를 계산합니다.
	// 거리 = |(카메라->플레이어) X (카메라->타겟)| / |카메라->타겟|
	const float Distance = FVector::CrossProduct(CamToPlayer, CamToTarget).Size() / CamToTarget.Size();

	// 계산된 거리가 설정된 임계값보다 크면 true를 반환합니다.
	return Distance > AlignmentResetThreshold;
}

FVector ADynamicCameraActor::GetAvoidanceAdjustedCameraLocation()
{
	FVector PlayerLocation = PlayerRef->GetActorLocation();
	FVector TargetLocation = TargetRef->GetActorLocation();

	// 1. 기본 목표 위치를 계산합니다 (플레이어의 등 뒤).
	FVector BaseCameraLocation = (PlayerLocation + (PlayerLocation - TargetLocation).GetSafeNormal() * 300) + FVector(0, 0, 100);

	// 2. 이 '기본 위치'에서 플레이어와 타겟이 일직선인지 확인합니다.
	FVector DirToPlayer = (PlayerLocation - BaseCameraLocation).GetSafeNormal();
	FVector DirToTarget = (TargetLocation - BaseCameraLocation).GetSafeNormal();
	float AlignmentDot = FVector::DotProduct(DirToPlayer, DirToTarget);

	// 3. 만약 일직선에 가깝다면 (Dot Product 결과가 임계값보다 크다면)
	if (AlignmentDot > ObstructionDotThreshold)
	{
		// 4. 카메라의 '오른쪽' 방향 벡터를 계산합니다.
		FVector RightVector = FVector::CrossProduct(DirToPlayer, FVector::UpVector).GetSafeNormal();

		// 5. 기본 위치에 '오른쪽' 벡터와 오프셋 거리를 곱한 값을 더해 목표 위치를 보정합니다.
		BaseCameraLocation += RightVector * ObstructionAvoidanceOffset;
	}

	// 6. 최종적으로 계산된 목표 위치를 반환합니다.
	return BaseCameraLocation;
}

void ADynamicCameraActor::OnDash(AActor* Target, bool IsDashing, FVector Direction)
{
	if ( PlayerRef != Target )
		return;

	const TCHAR* PrintMsg = IsDashing ? TEXT("Player Dashing Start") : TEXT("Player Dashing Complete");
	PRINTLOG(TEXT("%s"), PrintMsg);
}

void ADynamicCameraActor::OnTeleport(AActor* Target)
{
	if ( PlayerRef != Target )
		return;

	PRINTLOG(TEXT("OnTeleport"));
}

void ADynamicCameraActor::OnAttack(AActor* Target, int ComboCount)
{
	if ( PlayerRef != Target )
		return;

	PRINTLOG(TEXT("ComboCount : %d"), ComboCount);
}

void ADynamicCameraActor::OnSpecialAttack(AActor* Target, int32 SpecialIndex)
{
	if ( PlayerRef != Target )
		return;

	PRINTLOG(TEXT("OnSpecialAttack : %d"), SpecialIndex);
}

void ADynamicCameraActor::OnGuard(AActor* Target, bool bState)
{
	if ( PlayerRef != Target )
		return;
	
	const TCHAR* PrintMsg = bState ? TEXT("Player Guard Start") : TEXT("Player Guard End");
	PRINTLOG(TEXT("%s"), PrintMsg);
}

void ADynamicCameraActor::OnAvoid(AActor* Target, bool bState)
{
	if ( PlayerRef != Target )
		return;

	const TCHAR* PrintMsg = bState ? TEXT("Player Avoid Start") : TEXT("Player Avoid End");
	PRINTLOG(TEXT("%s"), PrintMsg);
}

void ADynamicCameraActor::OnPowerCharge(AActor* Target, bool bState)
{
	if ( PlayerRef != Target )
		return;
	
	const TCHAR* PrintMsg = bState ? TEXT("Player PowerCharge Start") : TEXT("Player PowerCharge End");
	PRINTLOG(TEXT("%s"), PrintMsg);
}