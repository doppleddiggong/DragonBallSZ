// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FKnockbackData.h"
#include "UKnockbackSystem.generated.h"


UCLASS( Blueprintable, ClassGroup=(DBSZ), meta=(BlueprintSpawnableComponent) )
class DRAGONBALLSZ_API UKnockbackSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UKnockbackSystem();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	 
    UFUNCTION(BlueprintCallable, Category="Knockback")
    void InitSystem(ACombatCharacter* InOwner);

    UFUNCTION(BlueprintCallable, Category="Knockback")
    void OnKnockback(AActor* Target, AActor* Instigator, EAttackPowerType Type, float Resistance);

private:
    static FVector ComputeKnockDir(const AActor* Target, const AActor* Instigator);
    void Knockback(AActor* Target, AActor* Instigator, EAttackPowerType Type, float Resistance);
    void RestoreMovement();
    void EnterFlying();

private:
	UPROPERTY()
	class ACombatCharacter* Owner;
	UPROPERTY()
	class USkeletalMeshComponent* MeshComp;
	UPROPERTY()
	class UCharacterMovementComponent* MoveComp;

	float PrevBrakingFriction = 1.f;
	bool  bFriction = false;
	FTimerHandle RestoreTimer;
    FTimerHandle FlyingTimer;
};
