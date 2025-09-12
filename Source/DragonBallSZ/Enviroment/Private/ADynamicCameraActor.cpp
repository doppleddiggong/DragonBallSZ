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

	CurrentDistance = FVector::Dist(PlayerRef->GetActorLocation(), TargetRef->GetActorLocation());
	// if (!PlayerRef->GetCharacterMovement()->IsFlying())
	// {
	// }

		PlayerRotationLock();
	if (CurrentDistance < TargetDistance)
	{
		CloseCameraRotation(DeltaTime);
		ResetCameraLocation(DeltaTime);
	}
	else
	{
		if (TargetDeadZoneCheck(*PlayerRef))
		{
			ResetCameraLocation(DeltaTime);
			if (TargetDeadZoneCheck(*TargetRef))
			{
				ResetCameraRotation(DeltaTime);
			}
		}
		else
		{
			ResetCameraForwardLocation(DeltaTime);
		}
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
	FVector PlayerLocation = PlayerRef->GetActorLocation();
	FVector TargetLocation = TargetRef->GetActorLocation();
	FVector NewLocation = UKismetMathLibrary::VInterpTo(GetActorLocation(), (PlayerLocation + (PlayerLocation - TargetLocation).GetSafeNormal() * 300) + FVector(0, 0, 100), DeltaTime, 5);
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

	bool result = UGameplayStatics::ProjectWorldToScreen(GetWorld()->GetFirstPlayerController(), Target.GetActorLocation(), ScreenPos, true );

	if (MinX < ScreenPos.X || MaxX > ScreenPos.X || MinY < ScreenPos.Y || MaxY > ScreenPos.Y)
	{
		return true;
	}
	return false;
}



void ADynamicCameraActor::OnDash(AActor* Target, bool IsDashing)
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