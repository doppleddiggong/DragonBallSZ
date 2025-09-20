// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "ECharacterType.h"
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

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Stats")
	FORCEINLINE bool IsDead()
	{
		return bIsDead;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Stats")
	FORCEINLINE float GetMaxHP()
	{
		return MaxHP;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Stats")
	FORCEINLINE float GetCurHP()
	{
		return CurHP;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Stats")
	FORCEINLINE float GetMaxKi()
	{
		return MaxKi;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Stats")
	FORCEINLINE float GetCurKi()
	{
		return CurKi;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Stats")
	FORCEINLINE bool IsBlastShotEnable()
	{
		return CurKi >= BlastNeedKi;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Stats")
	FORCEINLINE bool IsKamehameEnable()
	{
		return CurKi >= KamehameNeedKi;
	}
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Stats")
	FORCEINLINE float GetBlastShotDelay()
	{
		return BlastShotDelay;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Stats")
	FORCEINLINE float GetSightLength()
	{
		return SightLength;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Stats")
	FORCEINLINE float GetSightAngle()
	{
		return SightAngle;
	}	
	
	UFUNCTION(BlueprintCallable, Category="Stats")
	float GetRandDmg(float Damage);

	UFUNCTION(BlueprintCallable, Category="Stats")
	float GetAttackDamage(int ComboCount);

	UFUNCTION(BlueprintCallable, Category="Stats")
	float GetBlastDamage();
	UFUNCTION(BlueprintCallable, Category="Stats")
	float GetKamehameDamage();


	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Stats")
	void InitStat(const bool InIsPlayer, const ECharacterType InECharacterType);

	UFUNCTION(BlueprintCallable, Category="Stats")
	FORCEINLINE void IncreaseKi_ComboCount(int ComboCount)
	{
		if ( AttackChargeKi.IsValidIndex(ComboCount) )
			IncreaseKi(AttackChargeKi[ComboCount]);
	}

	UFUNCTION(BlueprintCallable, Category="Stats")
	FORCEINLINE void UseBlast()
	{
		DecreaseKi(BlastNeedKi);
	}

	UFUNCTION(BlueprintCallable, Category="Stats")
	FORCEINLINE void UseKamehame()
	{
		DecreaseKi(KamehameNeedKi);
	}
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Stats")
	void IncreaseHealth(float InHealPoint);	
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Stats")
	bool DecreaseHealth(float InDamagePoint);	

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Stats")
	void IncreaseKi(float InKi);	

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Stats")
	void DecreaseKi(float InKi);	

	
protected:
	// Health
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats|HP")
	float CurHP = 1000;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats|HP")
	float MaxHP = 1000;

	// Attack
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats")
	TArray<float> AttackDamage { 30, 45, 50, 70, 100 };
	// 공격시 회복기
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats")
	TArray<float> AttackChargeKi  { 3, 4, 5, 7, 10, 10, 10, 10, 10 };

	// Ki
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats|Ki")
	float CurKi = 300;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats|Ki")
	float MaxKi = 500;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats|Blast")
	float BlastNeedKi = 20;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats|Blast")
	float BlastDamage = 30;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats|Blast")
	float BlastShotDelay = 0.5;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats|Kamehame")
	float KamehameNeedKi = 100;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats|Kamehame")
	float KamehameDamage = 300;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats|Sight")
	float SightLength = 1000;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats|Sight")
	float SightAngle = 45;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats|State")
	ECharacterType CharacterType;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats|State")
	bool bIsPlayer = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats|State")
	bool bIsDead = false;
};
