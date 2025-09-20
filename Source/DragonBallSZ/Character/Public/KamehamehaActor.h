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
	TObjectPtr<class UNiagaraSystem> ExplosionSmoke;
	UPROPERTY()
	TObjectPtr<class UNiagaraComponent> ExplosionSmokeComp;
	
	UPROPERTY()
	TObjectPtr<class APostProcessVolume> PPVolume;
	
	UPROPERTY()
	TObjectPtr<class ACombatCharacter> Target;
	UPROPERTY()
	TObjectPtr<ACombatCharacter> Shooter;
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRecvMessage(FString InMsg);

public:	
	virtual void Tick(float DeltaTime) override;

	bool bFirstExplosion = false;
	bool bSecondExplosion = false;
	float ElapsedTime = 0;
	float LoopDuration = 0.37f;
	FVector BeamVector = FVector(1,1,1);
	float FirstExplosionTime = 3.9f;
	float SecondExplosionTime = 15.8f;
	float BeamSpeed = 2.f;
	UPROPERTY(EditDefaultsOnly, Category="Kamehameha Charge Speed")
	float LoopSpeed = 0.001;
	UPROPERTY(EditDefaultsOnly, Category="Kamehameha Impact Frame")
	float ImpactTime = 0.11f;

	UFUNCTION(BlueprintCallable)
	void FireKamehameha();
	UFUNCTION()
	void OnKamehamehaFinished(class UNiagaraComponent* PSystem);
	
	






public:
	bool StartKamehameAnim = false;

	UPROPERTY()
	ACombatCharacter* InOwner;
	UPROPERTY()
	ACombatCharacter* InTarget;
	
	UFUNCTION(BlueprintCallable, Category="Player")
	void StartKamehame(class ACombatCharacter* InKamehameOwner, class ACombatCharacter* InKamehameTarget);

	UFUNCTION(BlueprintCallable, Category="Player")
	void DelayKamehameFire();
	
	UFUNCTION(BlueprintCallable, Category="Player")
	void EndKamehame();

	UPROPERTY()
	class UDBSZEventManager* EventManager;
};
