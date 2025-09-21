// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AKamehamehaActor.generated.h"

UCLASS()
class DRAGONBALLSZ_API AKamehamehaActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AKamehamehaActor();

	UPROPERTY()
	TObjectPtr<class USceneComponent> RootComp;
	
	UPROPERTY(EditAnywhere, Category="VFX")
	TObjectPtr<class UNiagaraComponent> ChargeSphere;
	UPROPERTY(EditAnywhere, Category="VFX")
	TObjectPtr<class UNiagaraComponent> Kamehameha;
	UPROPERTY(EditAnywhere, Category="VFX")
	TObjectPtr<class UNiagaraComponent> FinishDust;
	UPROPERTY(EditAnywhere, Category="VFX")
	TObjectPtr<class UNiagaraSystem> Explosion;
	UPROPERTY(EditAnywhere, Category="VFX")
	TObjectPtr<class UNiagaraSystem> ExplosionSmoke;
	UPROPERTY()
	TObjectPtr<class UNiagaraComponent> ExplosionSmokeComp;
	UPROPERTY(EditAnywhere, Category="VFX")
	TObjectPtr<class UParticleSystem> ExplosionWind;
	
	UPROPERTY()
	TObjectPtr<class APostProcessVolume> PPVolume;
	
	UPROPERTY()
	TObjectPtr<class ACombatCharacter> Shooter;
	UPROPERTY()
	TObjectPtr<class ACombatCharacter> Target;

	UPROPERTY(EditAnywhere, Category="SFX")
	TObjectPtr<class UAudioComponent> KamehamehaContinueSFX;
	
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
	float SecondExplosionTime = 15.2f;
	float BeamSpeed = 2.f;
	UPROPERTY(EditDefaultsOnly, Category="Kamehameha Charge Speed")
	float LoopSpeed = 0.001;
	UPROPERTY(EditDefaultsOnly, Category="Kamehameha Impact Frame")
	float ImpactTime = 0.11f;

	FVector SecondBeamHeight = FVector(0,0,0);;
	float SecondBeamWidth = 0.0f;
	
	UFUNCTION(BlueprintCallable)
	void FireKamehameha();
	UFUNCTION()
	void OnKamehamehaFinished(class UNiagaraComponent* PSystem);
	

public:
	bool StartKamehameAnim = false;
	bool bTrackingOwnerHand = false;

	UFUNCTION(BlueprintCallable, Category="Player")
	void StartKamehame(class ACombatCharacter* InKamehameOwner, class ACombatCharacter* InKamehameTarget);

	UFUNCTION(BlueprintCallable, Category="Player")
	void DelayKamehameFire();
	
	UFUNCTION(BlueprintCallable, Category="Player")
	void EndKamehame();

	UFUNCTION(BlueprintCallable)
	void ClearKamehame();
	
	UPROPERTY()
	TObjectPtr<class UDBSZEventManager> EventManager;
};
