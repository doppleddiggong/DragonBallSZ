// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KamehamehaActor.generated.h"

UCLASS()
class DRAGONBALLSZ_API AKamehamehaActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AKamehamehaActor();

	UPROPERTY()
	TObjectPtr<class USceneComponent> RootComp;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UNiagaraComponent> ChargeSphere;
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UNiagaraComponent> Kamehameha;
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UNiagaraComponent> FinishDust;
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UNiagaraSystem> Explosion;
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UParticleSystem> ExplosionWind;

	UPROPERTY()
	TObjectPtr<ACharacter> Target;
	UPROPERTY()
	TObjectPtr<ACharacter> Shooter;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	bool bFirstExplosion = false;
	bool bSecondExplosion = false;
	float ElapsedTime = 0;
	float LoopDuration = 0.37f;
	FVector BeamVector = FVector(1,1,1);
	
	//ToDo: DataTable로 뽑기
	float LoopSpeed = 0.001;
	float BeamSpeed = 1.f;
	float FinisherTime = 5.f;

	UFUNCTION(BlueprintCallable)
	void FireKamehameha();
};
