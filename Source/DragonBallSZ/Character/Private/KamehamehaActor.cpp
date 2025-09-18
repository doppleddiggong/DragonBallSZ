// Fill out your copyright notice in the Description page of Project Settings.


#include "KamehamehaActor.h"

#include "AEnemyActor.h"
#include "APlayerActor.h"
#include "DragonBallSZ.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
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
		else if (!bFirstExplosion)
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
		
		if (!bFirstExplosion) return;
		
		ElapsedTime += DeltaTime;
		if (ElapsedTime > FinisherTime)
		{
			if (BeamVector.X > 0)
			{
				BeamVector.X -= BeamSpeed / 6;
				BeamVector.Z -= BeamSpeed / 6;
				Kamehameha->SetVariableVec3(FName("BeamVector"), BeamVector);
				//return;
			}
			else if (!bSecondExplosion)
			{
				Kamehameha->Deactivate();
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
		FString distStr = FString::Printf(TEXT("%f"), ElapsedTime);
		PRINTLOG(TEXT("%s"), *distStr);
		GEngine->AddOnScreenDebugMessage(4, 1, FColor::Cyan, distStr);
	}
}

void AKamehamehaActor::FireKamehameha()
{
	FRotator LookRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target->GetActorLocation());
	Kamehameha->SetWorldRotation(LookRot + FRotator(0, 90, 0));
	ChargeSphere->DeactivateImmediate();

	Kamehameha->Activate();
}
