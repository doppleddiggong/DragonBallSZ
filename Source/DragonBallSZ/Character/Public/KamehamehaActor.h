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

	UPROPERTY(EditAnywhere)
	TObjectPtr<class UNiagaraComponent> ChargeComp;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	float LoopDuration = 0.37;
	UPROPERTY(EditAnywhere)
	float LoopSpeed = 0.001;
};
