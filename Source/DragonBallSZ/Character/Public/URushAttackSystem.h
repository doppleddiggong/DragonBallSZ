// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "EAttackPowerType.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/ActorComponent.h"
#include "URushAttackSystem.generated.h"

UCLASS( Blueprintable, ClassGroup=(DBSZ), meta=(BlueprintSpawnableComponent) )
class DRAGONBALLSZ_API URushAttackSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	URushAttackSystem();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
						   FActorComponentTickFunction* ThisTickFunction) override;
	
private: // AnimNotify
	UFUNCTION()
	void OnMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& Payload);
	UFUNCTION()	
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void BindMontageDelegates(UAnimInstance* Anim);
	void UnbindMontageDelegates(UAnimInstance* Anim);

public:
	UFUNCTION(BlueprintCallable, Category="RushAttack")
	void InitSystem(class ACombatCharacter* InOwner, class UCharacterData* InData);

	UFUNCTION(BlueprintPure, Category="RushAttack")
	FORCEINLINE bool IsAttackEnable() const { return !bIsAttacking && !bIsDashing; }
	UFUNCTION(BlueprintPure, Category="RushAttack")
	FORCEINLINE bool IsAttackIng() const { return bIsAttacking || bIsDashing; }
	
	UFUNCTION(BlueprintPure, Category="RushAttack")
	FORCEINLINE bool ShouldLookAtTarget() const { return bIsAttacking || bIsDashing; }

public:
	UFUNCTION(BlueprintCallable, Category="RushAttack")
	void OnDashCompleted();
    UFUNCTION(BlueprintCallable, Category="RushAttack")
    void OnAttack();

	UFUNCTION(BlueprintCallable, Category="RushAttack")
	void StartAttackTrace();
	UFUNCTION(BlueprintCallable, Category="RushAttack")
	void StopAttackTrace();
	UFUNCTION(BlueprintCallable, Category="RushAttack")
	void AttackTrace();

	UFUNCTION(BlueprintCallable, Category="RushAttack")
	void PlayMontage(int32 MontageIndex);
    UFUNCTION(BlueprintCallable, Category="RushAttack")
    void DashToTarget(int32 MontageIndex);
	UFUNCTION(BlueprintCallable, Category="RushAttack")
	void TeleportToTarget(int32 MontageIndex);
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="RushAttack|Owner")
	class ACombatCharacter* Owner;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="RushAttack|Owner")
	class USkeletalMeshComponent* MeshComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="RushAttack|Owner")
	class UAnimInstance* AnimInstance;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="RushAttack|Owner")
	class UCharacterMovementComponent* MoveComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="RushAttack|Target")
	class ACombatCharacter* Target;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="RushAttack|Target")
	class UCharacterMovementComponent* TargetMoveComp;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|Debug")
	TEnumAsByte<EDrawDebugTrace::Type> DrawTraceState = EDrawDebugTrace::None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|Debug")
	float TraceLength  = 30.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|Debug")
	float TraceRadius  = 30.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|Debug")
	float TraceDrawTime = 1.5f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="RushAttack|Combo")
	bool bIsAttacking = false;
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|Combo")
	// float MinAttackDelay = 0.75f;	

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="RushAttack|Dash")
	bool bIsDashing = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|Dash", meta=(ClampMin="0.05", ClampMax="3.0", AllowPrivateAccess="true"))
    float DashDuration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|Engage", meta=(ClampMin="0.0", AllowPrivateAccess="true"))
    float AttackRange = 150.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|Engage", meta=(ClampMin="0.0", AllowPrivateAccess="true"))
	float DashEventRange = 450.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|Engage")
    float TeleportRange = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|Engage")
    float TeleportBehindOffset = 150.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|Engage")
    bool bTeleportAlignToGround = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|Engage", meta=(ClampMin="0"))
    float TeleportFlyZThreshold = 120.0f; 	// 텔레포트 시 Z 상승이 이 값보다 크면 비행 보조

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|AutoTrack", meta=(ClampMin="0", AllowPrivateAccess="true"))
	float AutoTrackTurnRateDeg = 540.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|AutoTrack", meta=(ClampMin="0", AllowPrivateAccess="true"))
	float AutoTrackMoveSpeed = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|ResetTime")
	float ComboResetTime_Offset = 1.0f;

private:
	UPROPERTY()
	class UDBSZEventManager* EventManager;

	UPROPERTY()
	TArray<TObjectPtr<UAnimMontage>> AttackMontages;
	TArray<EAttackPowerType> AttackPowerType;
	
	UPROPERTY()
	TObjectPtr<UAnimMontage> DashMontage;
	
private:
	float ElapsedTime = 0.0f;
	float LastAttackTime = 0.0f;
	bool bDelegatesBound = false;

	float ComboResetTime = 0.0f;
	int ComboCount = 0;

	int32 PendingMontageIndex = 0;
	TEnumAsByte<EMovementMode> PrevMovementMode;

	FTimerHandle KnockbackTimerHandler;
    FTimerHandle TraceTimeHandler;

	FVector DashStartLoc;
	FVector DashTargetLoc;
};
