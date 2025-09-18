// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "ACombatCharacter.h"
#include "AEnemyActor.generated.h"

UCLASS()
class DRAGONBALLSZ_API AEnemyActor : public ACombatCharacter
{
	GENERATED_BODY()

public:
	AEnemyActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void Landed(const FHitResult& Hit) override;
	
public:
	UFUNCTION(BlueprintCallable, Category="Avoid")
	void OnRestoreAvoid();
	// UFUNCTION(BlueprintCallable, Category="Event")
	// void OnDash(AActor* Target, bool IsDashing, FVector Direction);
	// UFUNCTION(BlueprintCallable, Category="Event")
	// void OnTeleport(AActor* Target);
	// UFUNCTION(BlueprintCallable, Category="Event")
	// void OnAttack(AActor* Target, int ComboCount);
	// UFUNCTION(BlueprintCallable, Category="Event")
	// void OnSpecialAttack(AActor* Target, int32 SpecialIndex);
	// UFUNCTION(BlueprintCallable, Category="Event")
	// void OnGuard(AActor* Target, bool bState);
	// UFUNCTION(BlueprintCallable, Category="Event")
	// void OnAvoid(AActor* Target, bool bState);
	// UFUNCTION(BlueprintCallable, Category="Event")
	// void OnPowerCharge(AActor* Target, bool bState);

	UFUNCTION(BlueprintCallable, Category="Enemy")
	void OnSightDetect(bool Target);

	
public: // Enemy Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy")
	class USightSystem* SightSystem;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy")
	class UEnemyFSM* EnemyFSM;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy")
	class UEnemyAnimInstance* AnimBP;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy")
	class AEnemyAIController* AIEnemy;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy")
	float RotateLerpSpeed = 5.0f;

private:
	UPROPERTY()
	class UDBSZEventManager* EventManager;
};
