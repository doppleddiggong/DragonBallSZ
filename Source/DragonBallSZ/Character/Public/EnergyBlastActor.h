// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnergyBlastActor.generated.h"

class UProjectileMovementComponent;
class APlayerActor;

UCLASS()
class DRAGONBALLSZ_API AEnergyBlastActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AEnergyBlastActor();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY()
	TObjectPtr<ACharacter> Target;
	UPROPERTY()
	TObjectPtr<ACharacter> Shooter;
	
	static AEnergyBlastActor* SpawnEnergyBlast(UWorld* World, ACharacter* Shooter, const FTransform& SpawnTransform);

	UPROPERTY(EditAnywhere)
	float Speed = 500;
};
