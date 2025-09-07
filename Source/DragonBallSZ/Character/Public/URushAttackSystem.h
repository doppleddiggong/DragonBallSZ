// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
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

private:
	UFUNCTION()
	void OnMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& Payload);
	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void BindMontageDelegates(UAnimInstance* Anim);
	void UnbindMontageDelegates(UAnimInstance* Anim);
	
public:
	UFUNCTION(BlueprintCallable, Category="RushAttack")
	void OnAttack();
	void PlayAttackByIndex(int32 Index);

	UFUNCTION(BlueprintCallable, Category="RushAttack")
	void StartAttackTrace();
	UFUNCTION(BlueprintCallable, Category="RushAttack")
	void StopAttackTrace();
	UFUNCTION(BlueprintCallable, Category="RushAttack")
	void AttackTrace();

	UFUNCTION(BlueprintCallable, Category="RushAttack")
	void GetHandLocation(USceneComponent* Hand, FVector& OutStart, FVector& OutEnd) const;
	UFUNCTION(BlueprintCallable, Category="RushAttack")
	void AttackSphereTrace(FVector Start, FVector End, float BaseDamage, AActor* DamageCauser);

	UFUNCTION(BlueprintCallable, Category="RushAttack")
	void ResetByHit();
	UFUNCTION(BlueprintCallable, Category="RushAttack")
	void ResetCounter();
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack")
	TArray<UAnimMontage*> AttackMontages;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack")
	class APlayerActor* Owner;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack")
	class USkeletalMeshComponent* MeshComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RushAttack")
	class UAnimInstance* AnimInstance;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="RushAttack")
	bool bIsAttacking = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="RushAttack")
	int ComboCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="RushAttack")
	float AttackSpeed = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="RushAttack")
	FTimerHandle AttackTimeHandler;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="RushAttack")
	float Damage = 30.0f;

	bool bDelegatesBound = false;
};
