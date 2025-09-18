// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EVFXType.h"
#include "GameFramework/Actor.h"
#include "AEnergyBlastActor.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UNiagaraSystem;
class UBoxComponent;
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

private:
	void HitProcess(AActor* DamagedActor, EVFXType VFXType);
	
public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<USphereComponent> SphereComp;
	UPROPERTY()
	TObjectPtr<class ACombatCharacter> Target;
	UPROPERTY()
	TObjectPtr<class ACombatCharacter> Shooter;
	
	float ElapsedTime;
	float SpeedUpTime = 0.6f;
	float Speed;
	float MinSpeed = 3700;
	float MaxSpeed = 5400;
};
