// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EBodyPartType.h"
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
	UFUNCTION(BlueprintPure, Category="Player|Sight")
	bool IsInSight(const AActor* Other) const;

	UFUNCTION(BlueprintPure, Category="CombatStart")
	FORCEINLINE bool IsCombatStart() const
	{
		return true;
		// return bIsCombatStart;
	};

	UFUNCTION(BlueprintCallable, Category="Command")
	void OnRecvMessage(const FString& InMsg);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Command")
	void OnLookTarget();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Command")
	void OnFlyEnd();

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player")
	class ACombatCharacter* TargetActor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player")
	bool IsHit = false;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|Arrow")
	class UArrowComponent* LeftHandComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|Arrow")
	class UArrowComponent* RightHandComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|Arrow")
	class UArrowComponent* LeftFootComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|Arrow")
	class UArrowComponent* RightFootComp;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|DashVFX")
	TObjectPtr<class UNiagaraSystem> DashNiagaraSystem = nullptr;

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
	
	FTimerHandle AvoidTimer;
	float AvoidTime = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Character|CombatStart")
	bool bIsCombatStart = false;
};
