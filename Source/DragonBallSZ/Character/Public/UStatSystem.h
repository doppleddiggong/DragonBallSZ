// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UStatSystem.generated.h"

UCLASS( Blueprintable, ClassGroup=(DBSZ), meta=(BlueprintSpawnableComponent) )
class DRAGONBALLSZ_API UStatSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UStatSystem();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;


	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Stats")
	FORCEINLINE bool IsPlayer()
	{
		return bIsPlayer;
	}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Stats")
	void InitStat(bool IsPlayer);	
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Stats")
	void IncreaseHealth(float InHealPoint);	
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Stats")
	bool DecreaseHealth(float InDamagePoint);	

	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	bool bIsPlayer = false;
	
	// Health
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float CurHP = 1000;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float MaxHP = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float Damage = 30;

	// STATE
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	bool IsDead = false;

	// Sight
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float SightLength = 1000;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float SightAngle = 45;
};
