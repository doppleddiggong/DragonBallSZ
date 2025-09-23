// Fill out your copyright notice in the Description page of Project Settings.


#include "AKamehamehaActor.h"

#include "ACombatCharacter.h"
#include "UDBSZEventManager.h"
#include "UDBSZFunctionLibrary.h"
#include "EAnimMontageType.h"

#include "EngineUtils.h"
#include "GameColor.h"
#include "GameEvent.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "UDBSZSoundManager.h"
#include "Components/AudioComponent.h"

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

	KamehamehaContinueSFX = CreateDefaultSubobject<UAudioComponent>(TEXT("KamehamehaComp"));
	KamehamehaContinueSFX->SetupAttachment(RootComponent);
	KamehamehaContinueSFX->bAutoActivate = false;
}

void AKamehamehaActor::BeginPlay()
{
	Super::BeginPlay();

	EventManager = UDBSZEventManager::Get(GetWorld());
	EventManager->OnMessage.AddDynamic(this, &AKamehamehaActor::OnRecvMessage);


	for (TActorIterator<APostProcessVolume> It(GetWorld()); It; ++It)
	{
		auto* PPV = *It;
		if (PPV && PPV->Tags.Contains(FName("PPVImpactFrame")))
		{
			PPVolume = PPV;
			break; // 찾으면 반복 종료
		}
	}
}

void AKamehamehaActor::OnRecvMessage(FString InMsg)
{
	if (InMsg.Equals(GameEvent::KameShoot.ToString(), ESearchCase::IgnoreCase))
	{
		DelayKamehameFire();
	}
}

void AKamehamehaActor::ClearKamehame()
{
	if (IsValid(ChargeSphere))
		ChargeSphere->DeactivateImmediate();

	if (IsValid(Kamehameha))
		Kamehameha->DeactivateImmediate();

	if (IsValid(FinishDust))
		FinishDust->DeactivateImmediate();
}

void AKamehamehaActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ElapsedTime += DeltaTime;

	if (LoopDuration > 0.1f)
	{
		LoopDuration -= LoopSpeed;

		ChargeSphere->SetVariableFloat(FName("LoopDuration"), LoopDuration);
	}


	if (bTrackingOwnerHand)
	{
		if (IsValid(Shooter))
		{
			this->SetActorLocation(Shooter->GetKamehameHandLocation());
		}
	}

	if (Kamehameha && Kamehameha->IsActive())
	{
		if (BeamVector.Y < 135)
		{
			BeamVector.Y += BeamSpeed;
			BeamVector.X += BeamSpeed / 5;
			BeamVector.Z += BeamSpeed / 5;
			Kamehameha->SetVariableVec3(FName("BeamVector"), BeamVector);
			SecondBeamHeight.Y -= BeamSpeed * 23;
			Kamehameha->SetVariableVec3(FName("BeamHeight"), SecondBeamHeight);
			SecondBeamWidth += BeamSpeed * 4.8f;
			Kamehameha->SetVariableFloat(FName("BeamWidth"), SecondBeamWidth);
			return;
		}

		if (ElapsedTime > FirstExplosionTime)
		{
			if (!bFirstExplosion)
			{
				UDBSZSoundManager::Get(GetWorld())->PlaySound2D(ESoundType::Kamehameha_Explosion);
		
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

				auto AttackPowerType = EAttackPowerType::Normal;
				EventManager->SendForceCameraShake(AttackPowerType);
				
				UGameplayStatics::ApplyDamage(
					Target,
					Shooter->GetKamehameDamage()*0.25f,
					nullptr,
					Owner,
					UDBSZFunctionLibrary::GetDamageTypeClass(AttackPowerType)
				);

				bFirstExplosion = true;
			}
		}

		if (!bFirstExplosion)
			return;

		if (ElapsedTime > SecondExplosionTime)
		{
			if (BeamVector.X > 0)
			{
				BeamVector.X -= BeamSpeed / 7;
				BeamVector.Z -= BeamSpeed / 7;
				Kamehameha->SetVariableVec3(FName("BeamVector"), BeamVector);
				SecondBeamWidth -= BeamSpeed * 3.1f;
				if (SecondBeamWidth <= 0.f) SecondBeamWidth = 0.f;
				Kamehameha->SetVariableFloat(FName("BeamWidth"), SecondBeamWidth);
			}
			else
			{
				Kamehameha->DeactivateImmediate();
			}

			if (!bSecondExplosion && BeamVector.X < 22)
			{
				KamehamehaContinueSFX->FadeOut(1.5f, 0.0f);
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

				UDBSZSoundManager::Get(GetWorld())->PlaySound2D(ESoundType::Kamehameha_Explosion);
				
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ExplosionWind,
					Target->GetActorLocation(),
					Target->GetActorRotation(),
					FVector(17.9f)
				);

				auto AttackPowerType = EAttackPowerType::Huge;
				EventManager->SendForceCameraShake(AttackPowerType);
				
				UGameplayStatics::ApplyDamage(
					Target,
					Shooter->GetKamehameDamage()*0.75f,
					nullptr,
					Owner,
					UDBSZFunctionLibrary::GetDamageTypeClass(AttackPowerType)
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

					UGameplayStatics::SpawnEmitterAtLocation(
						GetWorld(),
						ExplosionWind,
						Target->GetActorLocation(),
						Target->GetActorRotation(),
						FVector(7.7f)
					);
				}, 1.5f, false);

				bSecondExplosion = true;
			}
		}
	}
}

void AKamehamehaActor::FireKamehameha()
{
	auto& Blendables = PPVolume->Settings.WeightedBlendables.Array;

	if (Blendables.IsValidIndex(3))
	{
		FWeightedBlendable& PPVTarget = Blendables[3];
		PPVTarget.Weight = 1.f;
		
		PPVolume->Settings.WeightedBlendables.Array[3] = PPVTarget;
	}

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		[this]()
		{
			auto& Blendables = PPVolume->Settings.WeightedBlendables.Array;
			
			if (Blendables.IsValidIndex(3))
			{
				FWeightedBlendable& PPVTarget = Blendables[3];
				PPVTarget.Weight = 0.f;
		
				PPVolume->Settings.WeightedBlendables.Array[3] = PPVTarget;
			}
		},
		ImpactTime,
		false
	);

	FRotator LookRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target->GetActorLocation());
	FRotator OffsetRot(0.f, 90.f, 0.f);
	FQuat FinalRot = LookRot.Quaternion() * OffsetRot.Quaternion();
	Kamehameha->SetWorldRotation(FinalRot);
	FinishDust->SetWorldRotation(FinalRot);

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
	this->Shooter = InKamehameOwner;
	this->Target = InKamehameTarget;

	{
		StartKamehameAnim = true;
		bTrackingOwnerHand = true;
		// 발사 시작
		ChargeSphere->Activate();
		UDBSZSoundManager::Get(GetWorld())->PlaySound2D(ESoundType::Kamehameha_Charge);

		Shooter->PlayKamehame();

		// 발사자 발사하는 애니메이션 나온다
		// 나도 멈추고, 쟤도 멈춘다
		Shooter->SetHold(true);
		Target->SetHold(true);

		Shooter->UseKamehame();

		// 발사 준비 시작
		EventManager->SendSpecialAttack(Owner, 1);
		// 발사자 애니메이션 재생
		Shooter->PlayTypeMontage(EAnimMontageType::Kamehame);
		Shooter->SetOverlayMID(GameColor::Blue, 1.0f);


		// // 카메하메 애니메이션 총 프레임 딜레이
		// float PlayDelay = InOwner->KamehameMontage->GetPlayLength();
	}
}

void AKamehamehaActor::DelayKamehameFire()
{
	//일정 시간후에 발사한다
	// 발사 !!!!
	FireKamehameha();
	UDBSZSoundManager::Get(GetWorld())->PlaySound2D(ESoundType::Kamehameha_Fire);
	FTimerHandle TimerHandle;
	KamehamehaContinueSFX->Play();
	// USkeletalMeshComponent* Mesh = Shooter->GetMesh();
	// UAnimInstance* AnimInstance = Mesh->GetAnimInstance();
	// UAnimInstance* MyAnimBP = Cast<UAnimInstance>(AnimInstance);
	if (auto MyAnimBP = Shooter->GetAnimInstance())
	{
		if (MyAnimBP->Montage_IsPlaying(Shooter->KamehameMontage))
		{
			FName CurrentSection = MyAnimBP->Montage_GetCurrentSection(Shooter->KamehameMontage);
			if (CurrentSection == FName("Default"))
			{
				MyAnimBP->Montage_SetPlayRate(Shooter->KamehameMontage, 0.5);
			}
		}
	}

	Shooter->PlayKamehamePa();
	// 발사 시작할때 카메하페마 Shoot이벤트 송신
	//EventManager->SendSpecialAttack(Owner, 2);
}

void AKamehamehaActor::EndKamehame()
{
	if (StartKamehameAnim == false)
		return;

	StartKamehameAnim = false;
	bTrackingOwnerHand = false;

	// 발사 종료 3
	//발사 시작할때 카메하페마 Shoot이벤트 송신
	//EventManager->SendSpecialAttack(Owner, 3);
	Shooter->SetHold(false);
	Target->SetHold(false);

	Shooter->ClearKamehame();
	Shooter->SetOverlayMID(GameColor::Blue, 0.0f);
}
