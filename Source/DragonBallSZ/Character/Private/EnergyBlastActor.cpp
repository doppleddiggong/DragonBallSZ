// Fill out your copyright notice in the Description page of Project Settings.


#include "EnergyBlastActor.h"

#include "AEnemyActor.h"
#include "APlayerActor.h"
#include "DragonBallSZ.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AEnergyBlastActor::AEnergyBlastActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
}

// Called when the game starts or when spawned
void AEnergyBlastActor::BeginPlay()
{
	Super::BeginPlay();

	Shooter = Cast<ACharacter>(GetOwner());
	if (Shooter)
	{
		if (Shooter->IsA(APlayerActor::StaticClass()))
		{
			// Player가 쏜 경우 → Enemy를 타겟
			Target = Cast<ACharacter>(UGameplayStatics::GetActorOfClass(GetWorld(), AEnemyActor::StaticClass()));
			ensureMsgf(Target, TEXT("AEnergyBlastActor: Target 캐스팅 실패! AEnemyActor 필요합니다!"));
		}
		else if (Shooter->IsA(AEnemyActor::StaticClass()))
		{
			// Enemy가 쏜 경우 → Player를 타겟
			Target = Cast<APlayerActor>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
			ensureMsgf(Target, TEXT("UEnemyFSM: Target 캐스팅 실패! APlayerActor가 필요합니다!"));
		}
	}
}

// Called every frame
void AEnergyBlastActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	
	FVector Direction = Target->GetActorLocation() - this->GetActorLocation();
	Direction.Normalize();

	SetActorLocation(this->GetActorLocation() + Direction * DeltaTime * Speed);

	// 현재 상태 출력
	FVector TargetLoc = Target->GetActorLocation();
	FString LocStr = FString::Printf(TEXT("Target Location: X=%.2f Y=%.2f Z=%.2f"), 
							 TargetLoc.X, TargetLoc.Y, TargetLoc.Z);
	GEngine->AddOnScreenDebugMessage(2, 1, FColor::Cyan, LocStr);
}

AEnergyBlastActor* AEnergyBlastActor::SpawnEnergyBlast(UWorld* World, ACharacter* ThisOwner,const FTransform& SpawnTransform)
{
	if (!World || !ThisOwner) return nullptr;

	FActorSpawnParameters Params;
	Params.Owner = ThisOwner;
	Params.Instigator = ThisOwner;

	return World->SpawnActor<AEnergyBlastActor>(
		AEnergyBlastActor::StaticClass(),
		SpawnTransform,
		Params
	);
}
