// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EBodyPartType.h"
#include "ECharacterType.h"
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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
private:
	void BindMontageDelegates(UAnimInstance* Anim);
	void UnbindMontageDelegates(UAnimInstance* Anim);

	UFUNCTION()
	void OnMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& Payload);
	
public:
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

	UFUNCTION(BlueprintCallable, Category="Stats")
	FORCEINLINE float CurHP(float Per = 1.0f)
	{
		return StatSystem->GetCurHP() * Per;
	}
	
	UFUNCTION(BlueprintCallable, Category="Stats")
	FORCEINLINE float MaxHP(float Per = 1.0f)
	{
		return StatSystem->GetMaxHP() * Per;
	}
	
	UFUNCTION(BlueprintCallable, Category="Stats")
	FORCEINLINE float CurKi(float Per = 1.0f)
	{
		return StatSystem->GetCurKi() * Per;
	}

	UFUNCTION(BlueprintCallable, Category="Stats")
	FORCEINLINE float MaxKi(float Per = 1.0f)
	{
		return StatSystem->GetMaxKi() * Per;
	}
	
	UFUNCTION(BlueprintPure, Category="GameState")
	FORCEINLINE bool IsWinner() const
	{
		return bIsWinner;
	};

	UFUNCTION(BlueprintCallable, Category="CharacterType")
	FORCEINLINE ECharacterType GetCharacterType()
	{
		return CharacterType;
	}
	

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
	
	UFUNCTION(BlueprintCallable, Category="GameState")
	FORCEINLINE void SetChargeKi(const bool bState)
	{
		this->bIsChargeKi = bState;
	};

	UFUNCTION(BlueprintCallable, Category="GameState")
	FORCEINLINE bool IsChargeKi()
	{
		return bIsChargeKi;
	};

	
	UFUNCTION(BlueprintCallable, Category="GameState")
	FORCEINLINE bool IsShootKamehame()
	{
		return bIsShootKamehame;
	};

	UFUNCTION(BlueprintCallable, Category="Kamehame")
	FORCEINLINE void SetShootKamehame(const bool bState, class AKamehamehaActor* InKamehamehaActor )
	{
		this->bIsShootKamehame = bState;
		this->KamehamehaActor = InKamehamehaActor;
	};
	
	UFUNCTION(BlueprintPure, Category="Kamehame")
	FVector GetKamehameHandLocation() const;

	UFUNCTION(BlueprintCallable, Category="Overlay")
	void SetOverlayMID(const FLinearColor InColor, const float InValue);
	
public:
	UFUNCTION(BlueprintCallable, Category="Setup")
	void SetupCharacterFromType(const ECharacterType Type, const bool bIsAnother);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure, Category="Command")
	bool IsControlEnable();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure, Category="Command")
	bool IsMoveEnable();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure, Category="Command")
	bool IsAttackEnable();


	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure, Category="Command")
	bool IsDead();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure, Category="Command")
	bool IsHitting();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure, Category="Command")
	bool IsAttackIng();
	UFUNCTION(BlueprintPure, Category="Character|Sight")
	bool IsInSight(const AActor* Other) const;
	UFUNCTION(BlueprintCallable, Category="Character|Montage")
	UAnimMontage* GetRandomHitAnim();
	UFUNCTION(BlueprintCallable, Category="Character|Montage")
	UAnimMontage* GetRandomBlastAnim();
	
	UFUNCTION(BlueprintCallable, Category="Command")
	void OnRecvMessage(FString InMsg);

	UFUNCTION(BlueprintCallable, Category="Command")
	void OnPowerCharge(AActor* Target, bool bState);
	
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

	UFUNCTION(BlueprintCallable, Category="Command")
	void EnergyBlastShoot();

	UFUNCTION(BlueprintCallable, Category="Command")
	void KamehameShoot();

	UFUNCTION(BlueprintCallable, Category="Command")
	void ClearKamehame();

	UFUNCTION(BlueprintCallable, Category="Command")
	bool IsBlastShootEnable();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure, Category="Command")
	bool IsKamehameEnable();
	
	UFUNCTION(BlueprintCallable, Category="Stats")
	FORCEINLINE void SetAttackChargeKi(int ComboCount)
	{
		StatSystem->IncreaseKi_ComboCount(ComboCount);
	}
	
	UFUNCTION(BlueprintCallable, Category="Command")
	FORCEINLINE float GetBlastShootDelay()
	{
		return StatSystem->GetBlastShotDelay();
	}
	UFUNCTION(BlueprintCallable, Category="Stat")
	FORCEINLINE float GetAttackDamage(int ComboCount)
	{
		return StatSystem->GetAttackDamage(ComboCount);
	}

	UFUNCTION(BlueprintCallable, Category="Stat")
	FORCEINLINE float GetBlastDamage()
	{
		return StatSystem->GetBlastDamage();
	}

	UFUNCTION(BlueprintCallable, Category="Stat")
	FORCEINLINE float GetKamehameDamage()
	{
		return StatSystem->GetKamehameDamage();
	}

	UFUNCTION(BlueprintCallable, Category="Stats")
	FORCEINLINE void IncreaseKi(const float ChargeKiAmount)
	{
		StatSystem->IncreaseKi(ChargeKiAmount);
	}

	UFUNCTION(BlueprintCallable, Category="Stats")
	FORCEINLINE void UseBlast()
	{
		return StatSystem->UseBlast();
	}

	UFUNCTION(BlueprintCallable, Category="Stats")
	FORCEINLINE void UseKamehame()
	{
		return StatSystem->UseKamehame();
	}
	

public:
	UFUNCTION(BlueprintCallable, Category="Montage")
	FORCEINLINE UAnimInstance* GetAnimInstance()
	{
		return AnimInstance;
	}
	

	
	UFUNCTION(BlueprintCallable, Category="Montage")
	void PlayTypeMontage(const EAnimMontageType Type);

	UFUNCTION(BlueprintCallable, Category="Montage")
	void PlayTargetMontage(UAnimMontage* AnimMontage);

	UFUNCTION(BlueprintCallable, Category="Montage")
	void StopTargetMontage(const EAnimMontageType Type, const float BlendInOutTime);

	
public:
	UFUNCTION(BlueprintCallable, Category="Sound")
	void PlaySoundAttack();
	UFUNCTION(BlueprintCallable, Category="Sound")
	void PlaySoundHit();
	UFUNCTION(BlueprintCallable, Category="Sound")
	void PlaySoundJump();
	UFUNCTION(BlueprintCallable, Category="Sound")
	void PlaySoundTeleport();
	UFUNCTION(BlueprintCallable, Category="Sound")
	void PlaySoundWin();

public: // Combat Character ShaderComp
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|System")
	TObjectPtr<class UStatSystem> StatSystem;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|System")
	TObjectPtr<class UHitStopSystem> HitStopSystem;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|System")
	TObjectPtr<class UKnockbackSystem> KnockbackSystem;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|System")
	TObjectPtr<class URushAttackSystem> RushAttackSystem;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|System")
	TObjectPtr<class UDashSystem> DashSystem;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|System")
	TObjectPtr<class UFlySystem> FlySystem;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|System")
	TObjectPtr<class UChargeKiSystem> ChargeKiSystem;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character")
	TObjectPtr<class ACombatCharacter> TargetActor;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="RushAttack|Owner")
	TObjectPtr<class USkeletalMeshComponent> MeshComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="RushAttack|Owner")
	TObjectPtr<class UCharacterMovementComponent> MoveComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="RushAttack|Owner")
	TObjectPtr<class UAnimInstance> AnimInstance;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|Arrow")
	TObjectPtr<class UArrowComponent> LeftHandComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|Arrow")
	TObjectPtr<class UArrowComponent> RightHandComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|Arrow")
	TObjectPtr<class UArrowComponent> LeftFootComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|Arrow")
	TObjectPtr<class UArrowComponent> RightFootComp;

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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character")
	bool IsHit = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character")
	bool bIsHold = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character")
	bool bIsChargeKi = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character")
	bool bIsShootKamehame = false;


	FTimerHandle AvoidTimer;
	float AvoidTime = 1.0f;
	
protected:
	// 블라스트 마지막 발사 시간
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="EnergyBlastFactory")
	float LastBlastShotTime = 0;

	// // 경직 시간. HitEnd후 시간 보정
	// UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="EnergyBlastFactory")
	// float HitEndOffset = 0.15f;
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character|Data")
	TObjectPtr<class UCharacterData> CharacterData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|Montage")
	TArray<TObjectPtr<UAnimMontage>> HitMontages;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|Montage")
	TObjectPtr<class UAnimMontage> DeathMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|Montage")
	TArray<TObjectPtr<class UAnimMontage>> BlastMontages;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|Montage")
	TObjectPtr<class UAnimMontage> ChargeKiMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|Montage")
	TObjectPtr<class UAnimMontage> KamehameMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|Montage")
	TObjectPtr<class UAnimMontage> IntroMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|Montage")
	TObjectPtr<class UAnimMontage> WinMontage;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|DashVFX")
	TObjectPtr<class UNiagaraSystem> DashVFX;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|DashVFX")
	TObjectPtr<class UNiagaraSystem> ChargeKiVFX;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|DashVFX")
	TObjectPtr<class UNiagaraSystem> DashWindVFX;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|EnergyBlast")
	TSubclassOf<class AEnergyBlastActor> EnergyBlastFactory;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|Kamehameha")
	TSubclassOf<class AKamehamehaActor> KamehamehaFactory;
	
	UPROPERTY()
	TObjectPtr<class UMaterialInterface> OverlayMaterial;

	UPROPERTY()
	TObjectPtr<class UMaterialInstanceDynamic> OverlayMID;
	
protected:
	bool bDelegatesBound = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ECharacterType CharacterType;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TEnumAsByte<EMovementMode> PrevMoveMode;

	UPROPERTY()
	TObjectPtr<class AKamehamehaActor> KamehamehaActor;

	UPROPERTY()
	TObjectPtr<class UDBSZEventManager> EventManager;
};
