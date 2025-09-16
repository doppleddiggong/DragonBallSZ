// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EBodyPartType.h"
#include "UStatSystem.h"
#include "ACombatCharacter.generated.h"

UCLASS()
class DRAGONBALLSZ_API ACombatCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ACombatCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	FORCEINLINE UArrowComponent* GetBodyPart(EBodyPartType Part) const
	{
		switch (Part)
		{
		case EBodyPartType::Hand_L: return LeftHandComp;
		case EBodyPartType::Hand_R: return RightHandComp;
		case EBodyPartType::Foot_L: return LeftFootComp;
		case EBodyPartType::Foot_R: return RightFootComp;
		default:	return LeftHandComp;
		}
	}


	UFUNCTION(BlueprintPure, Category="GameState")
	FORCEINLINE bool IsCombatStart() const
	{
		return bIsCombatStart;
	};

	UFUNCTION(BlueprintPure, Category="GameState")
	FORCEINLINE bool IsCombatResult() const
	{
		return bIsCombatResult;
	};

	UFUNCTION(BlueprintPure, Category="GameState")
	FORCEINLINE bool IsPlayer() const
	{
		return StatSystem->IsPlayer();
	};

	UFUNCTION(BlueprintPure, Category="GameState")
	FORCEINLINE bool IsEnemy() const
	{
		return StatSystem->IsPlayer() == false;
	};

	UFUNCTION(BlueprintPure, Category="GameState")
	FORCEINLINE bool IsWinner() const
	{
		return bIsWinner;
	};

	UFUNCTION(BlueprintPure, Category="GameState")
	FORCEINLINE bool IsHolding() const
	{
		return bIsHold;
	};

	UFUNCTION(BlueprintCallable, Category="GameState")
	FORCEINLINE void SetHold(const bool bState)
	{
		this->bIsHold = bState;
	};
	
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure, Category="Command")
	bool IsControlEnable();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure, Category="Command")
	bool IsMoveEnable();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure, Category="Command")
	bool IsAttackEnable();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure, Category="Command")
	bool IsHitting();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure, Category="Command")
	bool IsAttackIng();
	UFUNCTION(BlueprintPure, Category="Character|Sight")
	bool IsInSight(const AActor* Other) const;
	UFUNCTION(BlueprintCallable, Category="Character|Montage")
	UAnimMontage* GetRandomHitAnim();
	
	UFUNCTION(BlueprintCallable, Category="Command")
	void OnRecvMessage(FString InMsg);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Command")
	void OnLookTarget();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Command")
	void OnFlyEnd();

	UFUNCTION(BlueprintCallable, Category="Command")
	void OnDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);
	
	UFUNCTION(BlueprintCallable, Category="Fly")
	void SetFlying();
	UFUNCTION(BlueprintCallable, Category="Fly")
	void SetFallingToWalk();

	UFUNCTION(BlueprintCallable, Category="Command")
	void RecoveryMovementMode(const EMovementMode InMovementMode);
	
public: // Combat Character ShaderComp
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|System")
	class UStatSystem* StatSystem;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|System")
	class UHitStopSystem* HitStopSystem;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|System")
	class UKnockbackSystem* KnockbackSystem;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|System")
	class URushAttackSystem* RushAttackSystem;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|System")
	class UDashSystem* DashSystem;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|System")
	class UFlySystem* FlySystem;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character")
	class ACombatCharacter* TargetActor;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="RushAttack|Owner")
	class USkeletalMeshComponent* MeshComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="RushAttack|Owner")
	class UAnimInstance* AnimInstance;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|Arrow")
	class UArrowComponent* LeftHandComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|Arrow")
	class UArrowComponent* RightHandComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|Arrow")
	class UArrowComponent* LeftFootComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|Arrow")
	class UArrowComponent* RightFootComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character|Sight", meta=(ClampMin="0"))
	float SightRange = 1200.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character|Sight", meta=(ClampMin="0", ClampMax="180"))
	float SightHalfFOVDeg = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character|Trace")
	float TraceLength  = 30.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character|Trace")
	float TraceRadius  = 30.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character|Trace")
	float TraceDrawTime = 1.5f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Character|CombatState")
	bool bIsCombatStart = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Character|CombatState")
	bool bIsCombatResult = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Character|CombatState")
	bool bIsWinner = false;


	FTimerHandle AvoidTimer;
	float AvoidTime = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character")
	bool IsHit = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character")
	bool bIsHold = false;
		
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character|Data")
	class UCharacterData* CharacterData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|EnergyBlast")
	TSubclassOf<class AEnergyBlastActor> EnergyBlastFactory;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|DashVFX")
	TObjectPtr<class UNiagaraSystem> DashVFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|Montage")
	TArray<TObjectPtr<UAnimMontage>> HitMontages;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|Montage")
	TObjectPtr<UAnimMontage> DeathMontage;
};
