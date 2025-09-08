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

	bUseControllerRotationYaw = true;
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
		Move->bOrientRotationToMovement = false;
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

void APlayerActor::Cmd_Move(const FVector2D& Axis)
{
	if (Controller)
	{
		const FRotator ControlRot = Controller->GetControlRotation();
		const FRotator YawRot(0.f, ControlRot.Yaw, 0.f);

		const FVector Forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
		const FVector Right   = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

		AddMovementInput(Forward, Axis.Y);
		AddMovementInput(Right,   Axis.X);
	}
}

void APlayerActor::Cmd_Look(const FVector2D& Axis)
{
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

void APlayerActor::Cmd_Jump()
{
	Jump();
}

void APlayerActor::Cmd_Dash()
{
	PRINTINFO();
}

void APlayerActor::Cmd_LockOn()
{
	PRINTINFO();
}

void APlayerActor::Cmd_ChargeKi(bool bPressed)
{
	PRINTINFO();
}

void APlayerActor::Cmd_Guard(bool bPressed)
{
	PRINTINFO();
}

void APlayerActor::Cmd_Vanish()
{
	PRINTINFO();
}

void APlayerActor::Cmd_RushAttack()
{
	PRINTINFO();
	RushAttackSystem->OnAttack();
}

void APlayerActor::Cmd_EnergyBlast()
{
	PRINTINFO();
}

void APlayerActor::Cmd_Kamehameha()
{
	PRINTINFO();
}
