// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnergyBlastActor.generated.h"

class USphereComponent;
class UNiagaraSystem;
class UBoxComponent;
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
	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<USphereComponent> SphereComp;
	UPROPERTY()
	TObjectPtr<ACharacter> Target;
	UPROPERTY()
	TObjectPtr<ACharacter> Shooter;
	
	UPROPERTY(EditAnywhere, Category = "VFX")
	TObjectPtr<UNiagaraSystem> Explosion; 
	
	UPROPERTY(EditAnywhere)
	float Speed = 4000;
};
