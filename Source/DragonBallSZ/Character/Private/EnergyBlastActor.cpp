// Fill out your copyright notice in the Description page of Project Settings.


#include "EnergyBlastActor.h"

#include "AEnemyActor.h"
#include "APlayerActor.h"
#include "DragonBallSZ.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AEnergyBlastActor::AEnergyBlastActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	SetRootComponent(BoxComp);

	BoxComp->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	BoxComp->SetGenerateOverlapEvents(true);
	BoxComp->SetCollisionProfileName(TEXT(""));
}

void AEnergyBlastActor::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                  const FHitResult& SweepResult)
{
	if (Shooter->IsA(APlayerActor::StaticClass()))
	{
		if (auto* TargetActor = Cast<AEnemyActor>(OtherActor))
		{
			FString Str = FString::Printf(TEXT("Target: %s"), *TargetActor->GetName());
			PRINTLOG(TEXT("%s"), *Str);
			GEngine->AddOnScreenDebugMessage(1, 1, FColor::Cyan, Str);
			this->Destroy();
		}
	}
	else if (Shooter->IsA(AEnemyActor::StaticClass()))
	{
		if (auto* TargetActor = Cast<APlayerActor>(OtherActor))
		{
			FString Str = FString::Printf(TEXT("Target: %s"), *TargetActor->GetName());
			PRINTLOG(TEXT("%s"), *Str);
			GEngine->AddOnScreenDebugMessage(1, 1, FColor::Cyan, Str);

			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Explosion, this->GetActorLocation(),
			                                         this->GetActorRotation(), FVector(1.1, 1.1, 1.1), true);
			this->Destroy();
		}
	}
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

	BoxComp->OnComponentBeginOverlap.AddDynamic(this, &AEnergyBlastActor::OnOverlap);
}

// Called every frame
void AEnergyBlastActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Direction = Target->GetActorLocation() - this->GetActorLocation();
	Direction.Normalize();

	SetActorLocation(this->GetActorLocation() + Direction * DeltaTime * Speed);
}
