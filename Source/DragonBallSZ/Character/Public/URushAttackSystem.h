// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "EBodyPartType.h"
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
	void InitSystem(class APlayerActor* InOwner);

	UFUNCTION(BlueprintCallable, Category="RushAttack")
	FORCEINLINE void SetDamage(float InDamage) { this->Damage = InDamage; }
	FORCEINLINE void ResetCounter()	{ ComboCount = 0; }
	
public:
	UFUNCTION(BlueprintCallable, Category="RushAttack")
	void OnLookTarget();
	
    UFUNCTION(BlueprintCallable, Category="RushAttack")
    void OnAttack();
	UFUNCTION(BlueprintCallable, Category="RushAttack")
	void PlayMontage(int32 MontageIndex);

	UFUNCTION(BlueprintCallable, Category="RushAttack")
	void StartAttackTrace();
	UFUNCTION(BlueprintCallable, Category="RushAttack")
	void StopAttackTrace();
	UFUNCTION(BlueprintCallable, Category="RushAttack")
	void AttackTrace();

	UFUNCTION(BlueprintCallable, Category="RushAttack")
	void GetBodyLocation(USceneComponent* SceneComp, FVector& OutStart, FVector& OutEnd) const;
	UFUNCTION(BlueprintCallable, Category="RushAttack")
	void AttackSphereTrace(FVector Start, FVector End, float BaseDamage, AActor* DamageCauser);

    UFUNCTION(BlueprintCallable, Category="RushAttack")
    void ResetByHit();

	
    UFUNCTION(BlueprintCallable, Category="RushAttack|Rush")
    void DashToAttackTarget(int32 MontageIndex);
    UFUNCTION()
    void OnDashCompleted();
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="RushAttack|Owner")
	class APlayerActor* Owner;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="RushAttack|Owner")
	class USkeletalMeshComponent* MeshComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="RushAttack|Owner")
	class UAnimInstance* AnimInstance;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="RushAttack|Owner")
	class UCharacterMovementComponent* MoveComp;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|Debug")
	TEnumAsByte<EDrawDebugTrace::Type> DrawTraceState = EDrawDebugTrace::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|Debug")
	float TraceLength  = 30.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|Debug")
	float TraceRadius  = 30.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|Debug")
	float TraceDrawTime = 1.5f;
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|Combo")
	TArray<EBodyPartType> AttackPart;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|Combo")
	TArray<UAnimMontage*> AttackMontages;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|Combo")
	TArray<EAttackPowerType> AttackPowerType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|Combo")
	float ComboAttackTime = 1.0f;
	

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="RushAttack|Combo")
	bool bIsAttacking = false;
	
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="RushAttack|Combo")
    int ComboCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="RushAttack|Dash")
	bool bIsDashing = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|Dash")
	UAnimMontage* DashMontages = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="RushAttack|Dash", meta=(AllowPrivateAccess="true"))
	bool bUseDashMove = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|Dash", meta=(ClampMin="0.05", ClampMax="3.0", AllowPrivateAccess="true"))
    float DashDuration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|Dash", meta=(ClampMin="0.0", AllowPrivateAccess="true"))
    float AttackRange = 150.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|Engage")
    float TeleportRange = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|Engage")
    float TeleportBehindOffset = 150.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|Engage")
    float TeleportThresholdMultiplier = 2.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|Engage")
    bool bTeleportAlignToGround = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|Engage")
    float TeleportHeightOffset = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|Engage")
    float AirAssistDuration = 0.4f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|Engage", meta=(ClampMin="0"))
    float TeleportFlyZThreshold = 120.0f; // 텔레포트 시 Z 상승이 이 값보다 크면 비행 보조
	
private:
	float Damage = 30.0f;
	bool bDelegatesBound = false;

    FTimerHandle ComboTimeHandler;
    FTimerHandle AttackTraceTimeHandler;

	TEnumAsByte<EMovementMode> PrevMovementMode;
	
	FVector DashStartLocation;
	FVector DashTargetLocation;
	float DashElapsedTime = 0.0f;

    int32 PendingMontageIndex = 0;

private:
    void EngageTarget(int32 MontageIndex);
    void TeleportAttackTarget(int32 MontageIndex);
    FTimerHandle AirAssistTimer;
};
