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

	UFUNCTION(BlueprintCallable, Category="Stats")
	float GetRandDmg(float Damage);

	UFUNCTION(BlueprintCallable, Category="Stats")
	float GetAttackDamage(int ComboCount);

	UFUNCTION(BlueprintCallable, Category="Stats")
	float GetBlastDamage();
	
	UFUNCTION(BlueprintCallable, Category="Stats")
	FORCEINLINE void SetAttackChargeKi(int ComboCount)
	{
		if ( ChargeKi.IsValidIndex(ComboCount) )
			IncreaseKi(ChargeKi[ComboCount]);
	}

	UFUNCTION(BlueprintCallable, Category="Stats")
	FORCEINLINE void UseBlast()
	{
		DecreaseKi(BlastNeedKi);
	}
	
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Stats")
	void IncreaseHealth(float InHealPoint);	
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Stats")
	bool DecreaseHealth(float InDamagePoint);	

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Stats")
	void IncreaseKi(float InKi);	

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Stats")
	void DecreaseKi(float InKi);	

	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|State")
	bool bIsPlayer = false;
	
	// Health
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|HP")
	float CurHP = 1000;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|HP")
	float MaxHP = 1000;

	// Attack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	TArray<float> AttackDamage { 30, 45, 50, 70, 100 };
	
	// Ki
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Ki")
	float CurKi = 300;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Ki")
	float MaxKi = 500;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Ki")
	float BlastNeedKi = 20;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Ki")
	float BlastDamage = 30;
	// 에너지탄 발사 딜레이
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EnergyBlastFactory")
	float BlastShotDelay = 0.5;
	
	// 공격시 회복기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	TArray<float> ChargeKi  { 3, 4, 5, 7, 10 };
	
	// STATE
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	bool IsDead = false;

	// Sight
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float SightLength = 1000;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	float SightAngle = 45;
};
