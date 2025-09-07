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


	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Stats")
	void InitStat();	
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Stats")
	void IncreaseHealth(float InHealPoint);	
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Stats")
	bool DecreaseHealth(float InDamagePoint);	

public:
	// Health
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float CurHP;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float MaxHP;

	// STATE
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	bool IsDead;

	// Sight
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float SightLength;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float SightAngle;
};
