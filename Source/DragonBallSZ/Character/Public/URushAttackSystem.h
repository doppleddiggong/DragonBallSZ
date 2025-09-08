// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "EBodyPartType.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/ActorComponent.h"
#include "URushAttackSystem.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DRAGONBALLSZ_API URushAttackSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	URushAttackSystem();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

private: // AnimNotify
	UFUNCTION()
	void OnMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& Payload);
	UFUNCTION()	
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void BindMontageDelegates(UAnimInstance* Anim);
	void UnbindMontageDelegates(UAnimInstance* Anim);
	
public:
	UFUNCTION(BlueprintCallable, Category="RushAttack")
	FORCEINLINE void SetDamage(float InDamage) { this->Damage = InDamage; }
	FORCEINLINE void ResetCounter()	{ ComboCount = 0; }
	
public:
	UFUNCTION(BlueprintCallable, Category="RushAttack")
	void OnLookTarget();
	
    UFUNCTION(BlueprintCallable, Category="RushAttack")
    void OnAttack();
	UFUNCTION(BlueprintCallable, Category="RushAttack")
	void PlayAttackMontage(int32 MontageIndex);

	UFUNCTION(BlueprintCallable, Category="RushAttack")
	void StartAttackTrace();
	UFUNCTION(BlueprintCallable, Category="RushAttack")
	void StopAttackTrace();
	UFUNCTION(BlueprintCallable, Category="RushAttack")
	void AttackTrace();

	UFUNCTION(BlueprintCallable, Category="RushAttack")
	void GetBodyLocation(USceneComponent* Hand, FVector& OutStart, FVector& OutEnd) const;
	UFUNCTION(BlueprintCallable, Category="RushAttack")
	void AttackSphereTrace(FVector Start, FVector End, float BaseDamage, AActor* DamageCauser);

    UFUNCTION(BlueprintCallable, Category="RushAttack")
    void ResetByHit();

    UFUNCTION(BlueprintCallable, Category="RushAttack|Rush")
    void StartRushToTarget(int32 MontageIndex);
    UFUNCTION()
    void OnRushDashCompleted();
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="RushAttack|Owner")
	class APlayerActor* Owner;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="RushAttack|Owner")
	class USkeletalMeshComponent* MeshComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="RushAttack|Owner")
	class UAnimInstance* AnimInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|Debug")
	TEnumAsByte<EDrawDebugTrace::Type> DrawTraceState = EDrawDebugTrace::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|Combo")
	TArray<EBodyPartType> AttackPart;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|Combo")
	TArray<UAnimMontage*> AttackMontages;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="RushAttack|Combo")
	bool bIsAttacking = false;
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="RushAttack|Combo")
    int ComboCount = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|Dash")
	UAnimMontage* DashMontages = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="RushAttack|Dash", meta=(AllowPrivateAccess="true"))
	bool bUseDashMove = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|Dash", meta=(ClampMin="0.05", ClampMax="3.0", AllowPrivateAccess="true"))
    float DashDuration = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack|Dash", meta=(ClampMin="0.0", AllowPrivateAccess="true"))
	float DashStopDistance = 150.0f;
	
private:
	float Damage = 30.0f;
	bool bDelegatesBound = false;

    FTimerHandle ComboTimeHandler;
    FTimerHandle AttackTraceTimeHandler;

	bool bIsDashing = false;
	int32 PendingMontageIndex = 0;
};