// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FHitStopData.h"
#include "UHitStopSystem.generated.h"

UCLASS( Blueprintable, ClassGroup=(DBSZ), meta=(BlueprintSpawnableComponent) )
class DRAGONBALLSZ_API UHitStopSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UHitStopSystem();

public:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category="HitStop")
	void InitSystem(ACombatCharacter* InOwner);

	UFUNCTION(BlueprintCallable, Category="HitStop")
	void OnHitStop(AActor* Target, const EAttackPowerType Type);
	
	UFUNCTION(BlueprintCallable, Category="HitStop")
	void ApplyHitStop(const EAttackPowerType Type);
	
private:
	void BeginFreeze(const FHitStopData& Params);
	void EndFreeze();

private:
	UPROPERTY()
	class ACombatCharacter* Owner;
	UPROPERTY()
	class USkeletalMeshComponent* MeshComp;
	UPROPERTY()
	class UCharacterMovementComponent* MoveComp;

	FHitStopData LastParams;
	
	bool bActive = false;
	double EndRealTimeSeconds = 0.0;

	float SavedCustomTimeDilation = 1.0f;
};
