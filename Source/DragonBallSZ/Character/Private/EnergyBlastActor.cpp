// Fill out your copyright notice in the Description page of Project Settings.


#include "EnergyBlastActor.h"

#include "AEnemyActor.h"
#include "APlayerActor.h"
#include "DragonBallSZ.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SphereComponent.h"

#include "EAttackPowerType.h"
#include "UDBSZEventManager.h"
#include "UDBSZVFXManager.h"


// Sets default values
AEnergyBlastActor::AEnergyBlastActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SetRootComponent(SphereComp);

	SphereComp->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	SphereComp->SetGenerateOverlapEvents(true);
	SphereComp->SetCollisionProfileName(TEXT(""));
}

void AEnergyBlastActor::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                  const FHitResult& SweepResult)
{
	if (Shooter->IsA(APlayerActor::StaticClass()))
	{
		if (auto* TargetActor = Cast<AEnemyActor>(OtherActor))	// Overlapping Enemy
		{
			// SpawnExplosionVFX();
			this->Destroy();
		}
	}
	else if (Shooter->IsA(AEnemyActor::StaticClass()))
	{
		if (auto* TargetActor = Cast<APlayerActor>(OtherActor))	// Overlapping Player
		{
			// SpawnExplosionVFX();
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
		if (Shooter->IsA(APlayerActor::StaticClass()))	// Shooter: Player
		{
			// Target: Enemy
			Target = Cast<ACharacter>(UGameplayStatics::GetActorOfClass(GetWorld(), AEnemyActor::StaticClass()));
			ensureMsgf(Target, TEXT("AEnergyBlastActor: Target 캐스팅 실패! AEnemyActor 필요합니다!"));
		}
		else if (Shooter->IsA(AEnemyActor::StaticClass()))	// Shooter: Enemy
		{
			// Target: Player
			Target = Cast<APlayerActor>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
			ensureMsgf(Target, TEXT("UEnemyFSM: Target 캐스팅 실패! APlayerActor가 필요합니다!"));
		}
	}

	SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AEnergyBlastActor::OnOverlap);
}

// Called every frame
void AEnergyBlastActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Direction = Target->GetActorLocation() - this->GetActorLocation();
	Direction.Normalize();

	SetActorLocation(this->GetActorLocation() + Direction * DeltaTime * Speed);
}
//
// void AEnergyBlastActor::SpawnExplosionVFX()
// {
// 	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
// 				GetWorld(),
// 				Explosion,
// 				GetActorLocation(),
// 				GetActorRotation(),
// 				FVector(0.4f, 0.4f, 0.4f),
// 				true,
// 				true,
// 				ENCPoolMethod::None,
// 				true
// 			);
// }

void AEnergyBlastActor::HitProcess(AActor* DamagedActor, EVFXType VFXType)
{
	UDBSZVFXManager::Get(GetWorld())->ShowVFX(
		VFXType,
		GetActorLocation(), GetActorRotation(),FVector(0.4f) );

	const EAttackPowerType Type = EAttackPowerType::Small;
	if( auto EventManager = UDBSZEventManager::Get(GetWorld()) )
	{
		EventManager->SendHitStop(DamagedActor, Type);
		EventManager->SendKnockback(DamagedActor, this->Owner, Type, 0.3f);
	}
	
	UGameplayStatics::ApplyDamage(
		DamagedActor,
		Damage,
		nullptr,
		Owner,
		UDamageType::StaticClass()
	);
}