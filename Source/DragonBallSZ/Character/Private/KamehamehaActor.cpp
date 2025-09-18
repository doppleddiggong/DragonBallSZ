// Fill out your copyright notice in the Description page of Project Settings.


#include "KamehamehaActor.h"

#include "AEnemyActor.h"
#include "APlayerActor.h"
#include "DragonBallSZ.h"
#include "EAnimMontageType.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "UDBSZEventManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AKamehamehaActor::AKamehamehaActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp1"));
	RootComponent = RootComp;

	Kamehameha = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Kamehameha"));
	Kamehameha->SetupAttachment(RootComponent);
	Kamehameha->bAutoActivate = false;

	ChargeSphere = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ChargeSphere"));
	ChargeSphere->SetupAttachment(RootComponent);
	ChargeSphere->bAutoActivate = true;

	FinishDust = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FinishDust"));
	FinishDust->SetupAttachment(RootComponent);
	FinishDust->bAutoActivate = false;
}

// Called when the game starts or when spawned
void AKamehamehaActor::BeginPlay()
{
	Super::BeginPlay();

	// Shooter = Cast<ACharacter>(GetOwner());
	// if (Shooter)
	// {
	// 	if (Shooter->IsA(APlayerActor::StaticClass())) // Shooter: Player
	// 	{
	// 		// Target: Enemy
	// 		Target = Cast<ACharacter>(UGameplayStatics::GetActorOfClass(GetWorld(), AEnemyActor::StaticClass()));
	// 		ensureMsgf(Target, TEXT("AEnergyBlastActor: Target 캐스팅 실패! AEnemyActor 필요합니다!"));
	// 	}
	// 	else if (Shooter->IsA(AEnemyActor::StaticClass())) // Shooter: Enemy
	// 	{
	// 		// Target: Player
	// 		Target = Cast<APlayerActor>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	// 		ensureMsgf(Target, TEXT("UEnemyFSM: Target 캐스팅 실패! APlayerActor가 필요합니다!"));
	// 	}
	// }

	Target = Cast<APlayerActor>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	EventManager = UDBSZEventManager::Get(GetWorld());
}

// Called every frame
void AKamehamehaActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (LoopDuration > 0.1f)
	{
		LoopDuration -= LoopSpeed;

		ChargeSphere->SetVariableFloat(FName("LoopDuration"), LoopDuration);
	}

	if (Kamehameha && Kamehameha->IsActive())
	{
		if (BeamVector.Y < 135)
		{
			BeamVector.Y += BeamSpeed;
			BeamVector.X += BeamSpeed / 6;
			BeamVector.Z += BeamSpeed / 6;
			Kamehameha->SetVariableVec3(FName("BeamVector"), BeamVector);
			return;
		}

		ElapsedTime += DeltaTime;
		if (ElapsedTime > FirstExplosionTime)
		{
			if (!bFirstExplosion)
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(
					GetWorld(),
					Explosion,
					Target->GetActorLocation(),
					Target->GetActorRotation(),
					FVector(1.f),
					true,
					true,
					ENCPoolMethod::None,
					true
				);
				bFirstExplosion = true;
			}
		}
		
		if (!bFirstExplosion) return;
		
		if (ElapsedTime > SecondExplosionTime)
		{
			if (BeamVector.X > 0)
			{
				BeamVector.X -= BeamSpeed / 6;
				BeamVector.Z -= BeamSpeed / 6;
				Kamehameha->SetVariableVec3(FName("BeamVector"), BeamVector);
			}
			else
			{
				Kamehameha->DeactivateImmediate();
			}
			
			if (!bSecondExplosion && BeamVector.X < 10)
			{
				FinishDust->Activate();
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(
					GetWorld(),
					Explosion,
					Target->GetActorLocation(),
					Target->GetActorRotation(),
					FVector(1.f),
					true,
					true,
					ENCPoolMethod::None,
					true
				);

				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ExplosionWind,
					Target->GetActorLocation(),
				Target->GetActorRotation(),
					FVector(1.f),
					true
				);
				bSecondExplosion = true;
			}
		}
		
		if (ElapsedTime > 10)
		{
			this->Destroy();
		}
	}
}

void AKamehamehaActor::FireKamehameha()
{
	FRotator LookRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target->GetActorLocation());
	Kamehameha->SetWorldRotation(LookRot + FRotator(0, 90, 0));
	FinishDust->SetWorldRotation(LookRot + FRotator(0, 90, 0));
	ChargeSphere->DeactivateImmediate();

	Kamehameha->Activate();
}



void AKamehamehaActor::StartKamehame(ACombatCharacter* InKamehameOwner, ACombatCharacter* InKamehameTarget)
{
	this->InOwner = InKamehameOwner;
	this->InTarget = InKamehameTarget; 

	{
		StartKamehameAnim = true;
		// 발사 시작
		// 발사자 발사하는 애니메이션 나온다
		// 나도 멈추고, 쟤도 멈춘다
		InOwner->SetHold(true);
		InTarget->SetHold(true);

		// 발사 준비 시작
		EventManager->SendSpecialAttack(Owner, 1);
		// 발사자 애니메이션 재생
		InOwner->PlayTypeMontage(EAnimMontageType::Kamehame);
		// 카메하메 애니메이션 총 프레임 딜레이
		float PlayDelay = InOwner->KamehameMontage->GetPlayLength();
	}

	//일정 시간후에 발사한다
	// DelayKamehameFire();
}

void AKamehamehaActor::DelayKamehameFire()
{
	//일정 시간후에 발사한다

	// 발사 !!!!
	// 발사 시작할때 카메하페마 Shoot이벤트 송신
	//EventManager->SendSpecialAttack(Owner, 2);
}

void AKamehamehaActor::EndKamehame()
{
	if ( StartKamehameAnim == false)
		return;

	StartKamehameAnim = false;

	// 발사 종료 3
	//발사 시작할때 카메하페마 Shoot이벤트 송신
	//EventManager->SendSpecialAttack(Owner, 3);
	InOwner->SetHold(false);
	InTarget->SetHold(false);
}