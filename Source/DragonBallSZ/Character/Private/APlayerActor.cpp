// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "APlayerActor.h"

#include "AEnemyActor.h"
#include "DragonBallSZ.h"
#include "UDBSZEventManager.h"
#include "URushAttackSystem.h"
#include "UStatSystem.h"
#include "Components/ArrowComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

APlayerActor::APlayerActor()
{
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, 270.0f, 0.0f));

	StatSystem		= CreateDefaultSubobject<UStatSystem>(TEXT("StatSystem"));
	RushAttackSystem = CreateDefaultSubobject<URushAttackSystem>(TEXT("RushAttackSystem"));

	LeftHandComp = CreateDefaultSubobject<UArrowComponent>(TEXT("LeftHandComp"));
	LeftHandComp->SetupAttachment(GetMesh(), TEXT("hand_l"));

	RightHandComp = CreateDefaultSubobject<UArrowComponent>(TEXT("RightHandComp"));
	RightHandComp->SetupAttachment(GetMesh(), TEXT("hand_r"));
	RightHandComp->SetRelativeRotation(FRotator(180.f, 0.f, 0.f));

	LeftFootComp = CreateDefaultSubobject<UArrowComponent>(TEXT("LeftFootComp"));
	LeftFootComp->SetupAttachment(GetMesh(), TEXT("foot_l"));
	LeftFootComp->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));

	RightFootComp = CreateDefaultSubobject<UArrowComponent>(TEXT("RightFootComp"));
	RightFootComp->SetupAttachment(GetMesh(), TEXT("foot_r"));
	RightHandComp->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	
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

	StatSystem->InitStat(true);
	RushAttackSystem->SetDamage( StatSystem->Damage );

	if (auto EventManager = UDBSZEventManager::Get(GetWorld()))
		EventManager->SendUpdateHealth(true, StatSystem->CurHP, StatSystem->MaxHP);
}

void APlayerActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerActor::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void APlayerActor::Cmd_Move_Implementation(const FVector2D& Axis)
{
	if (Controller)
	{
		// const FRotator ControlRot = Controller->GetControlRotation();
		// const FRotator YawRot(0.f, ControlRot.Yaw, 0.f);
		//
		// const FVector Forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
		// const FVector Right   = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

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
	Jump();
}

void APlayerActor::Cmd_Dash_Implementation()
{
	PRINTINFO();
}

void APlayerActor::Cmd_Landing_Implementation()
{
	if (GetCharacterMovement()->MovementMode == MOVE_Flying)
	{
		// 지면을 레이캐스트로 찾기
		FHitResult Hit;
		FVector Start = GetActorLocation();
		FVector End = Start - FVector(0, 0, 10000);

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
		{
			FVector LandingLocation = Hit.Location;
			LandingLocation.Z += 88.0f;

			SetActorLocation(LandingLocation);
		}

		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
}

void APlayerActor::Cmd_ChargeKi_Implementation(bool bPressed)
{
	PRINTINFO();
}

void APlayerActor::Cmd_Guard_Implementation(bool bPressed)
{
	PRINTINFO();
}

void APlayerActor::Cmd_Vanish_Implementation()
{
	PRINTINFO();
}

void APlayerActor::Cmd_RushAttack_Implementation()
{
	PRINTINFO();
	RushAttackSystem->OnAttack();
}

void APlayerActor::Cmd_EnergyBlast_Implementation()
{
	PRINTINFO();
	// 조기탄
}

void APlayerActor::Cmd_Kamehameha_Implementation()
{
	PRINTINFO();
}
