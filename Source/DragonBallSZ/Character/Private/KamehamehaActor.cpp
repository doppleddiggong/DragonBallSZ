// Fill out your copyright notice in the Description page of Project Settings.


#include "KamehamehaActor.h"

#include "DragonBallSZ.h"
#include "NiagaraComponent.h"

// Sets default values
AKamehamehaActor::AKamehamehaActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ChargeComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ChargeComp"));
}

// Called when the game starts or when spawned
void AKamehamehaActor::BeginPlay()
{
	Super::BeginPlay();

	ChargeComp->Activate();
}

// Called every frame
void AKamehamehaActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (LoopDuration > 0.09f)
	{
		LoopDuration -= LoopSpeed;
		FString distStr = FString::Printf(TEXT("%f"), LoopDuration);
		PRINTLOG(TEXT("%s"), *distStr);
		GEngine->AddOnScreenDebugMessage(0, 1, FColor::Cyan, distStr);
		
		ChargeComp->SetVariableFloat(FName("LoopDuration"), LoopDuration);
	}

	
}
