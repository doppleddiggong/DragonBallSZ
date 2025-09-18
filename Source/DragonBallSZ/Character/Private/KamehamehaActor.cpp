// Fill out your copyright notice in the Description page of Project Settings.


#include "KamehamehaActor.h"
#include "AEnemyActor.h"
#include "APlayerActor.h"
#include "EAnimMontageType.h"
#include "EVFXType.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "UDBSZEventManager.h"
#include "UDBSZFunctionLibrary.h"
#include "UDBSZVFXManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"

AKamehamehaActor::AKamehamehaActor()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp1"));
	RootComponent = RootComp;

	Kamehameha = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Kamehameha"));
	Kamehameha->SetupAttachment(RootComponent);
	Kamehameha->bAutoActivate = false;

	ChargeSphere = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ChargeSphere"));
	ChargeSphere->SetupAttachment(RootComponent);
	ChargeSphere->bAutoActivate = false;

	FinishDust = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FinishDust"));
	FinishDust->SetupAttachment(RootComponent);
	FinishDust->bAutoActivate = false;
}

void AKamehamehaActor::BeginPlay()
{
	Super::BeginPlay();

	EventManager = UDBSZEventManager::Get(GetWorld());
}

void AKamehamehaActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ElapsedTime += DeltaTime;

	if (Kamehameha && Kamehameha->IsActive())
	{
		if (BeamVector.Y < 135)
		{
			BeamVector.Y += BeamSpeed;
			BeamVector.X += BeamSpeed / 5;
			BeamVector.Z += BeamSpeed / 5;
			Kamehameha->SetVariableVec3(FName("BeamVector"), BeamVector);
			return;
		}

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

				UGameplayStatics::ApplyDamage(
					Target,
					Shooter->GetKamehameDamage(),
					nullptr,
					Owner,
					UDBSZFunctionLibrary::GetDamageTypeClass(EAttackPowerType::Normal)
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

			if (!bSecondExplosion && BeamVector.X < 14)
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
				
				UGameplayStatics::ApplyDamage(
					Target,
					Shooter->GetBlastDamage(),
					nullptr,
					Owner,
					UDBSZFunctionLibrary::GetDamageTypeClass(EAttackPowerType::Huge)
				);

				FTimerHandle TimerHandle;
				GetWorldTimerManager().SetTimer(TimerHandle, [this]()
				{
					ExplosionSmokeComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
						GetWorld(),
						ExplosionSmoke,
						Target->GetActorLocation(),
						Target->GetActorRotation(),
						FVector(1.f),
						true,
						true,
						ENCPoolMethod::None,
						true
					);
					if (ExplosionSmokeComp)
					{
						ExplosionSmokeComp->OnSystemFinished.AddDynamic(this, &AKamehamehaActor::OnKamehamehaFinished);
					}
				}, 1.5f, false);

				bSecondExplosion = true;
			}
		}
	}
}

void AKamehamehaActor::FireKamehameha()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	APostProcessVolume* PPVolume = GetWorld()->SpawnActor<APostProcessVolume>(
		APostProcessVolume::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	UMaterialInstanceDynamic* DynamicMat = UMaterialInstanceDynamic::Create(ImpactFrameMaterial, PPVolume);
	PPVolume->Settings.AddBlendable(DynamicMat, 1.0f);
	PPVolume->bUnbound = true; // Infinite Extent

	// 1초 뒤에 끄기 (Timer 이용)
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		[PPVolume]()
		{
			PPVolume->BlendWeight = 0.f;
			PPVolume->Destroy();
		},
		ImpactTime, // 1초 뒤
		false // 반복X
	);

	FRotator LookRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target->GetActorLocation());
	Kamehameha->SetWorldRotation(LookRot + FRotator(0, 90, 0));
	FinishDust->SetWorldRotation(LookRot + FRotator(0, 90, 0));
	ChargeSphere->DeactivateImmediate();

	Kamehameha->Activate();
}

void AKamehamehaActor::OnKamehamehaFinished(class UNiagaraComponent* PSystem)
{
	// 이펙트 끝나면 액터 자동 소멸
	EndKamehame();
	this->Destroy();
}


void AKamehamehaActor::StartKamehame(ACombatCharacter* InKamehameOwner, ACombatCharacter* InKamehameTarget)
{
	this->InOwner = InKamehameOwner;
	this->InTarget = InKamehameTarget;
	Shooter = InOwner;
	Target = InTarget;

	{
		StartKamehameAnim = true;
		// 발사 시작
		ChargeSphere->Activate();
		if (LoopDuration > 0.1f)
		{
			LoopDuration -= LoopSpeed;

			ChargeSphere->SetVariableFloat(FName("LoopDuration"), LoopDuration);
		}
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
	FireKamehameha();
	// 발사 시작할때 카메하페마 Shoot이벤트 송신
	//EventManager->SendSpecialAttack(Owner, 2);
}

void AKamehamehaActor::EndKamehame()
{
	if (StartKamehameAnim == false)
		return;

	StartKamehameAnim = false;

	// 발사 종료 3
	//발사 시작할때 카메하페마 Shoot이벤트 송신
	//EventManager->SendSpecialAttack(Owner, 3);
	InOwner->SetHold(false);
	InTarget->SetHold(false);
}