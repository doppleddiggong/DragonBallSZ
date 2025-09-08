// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AEnemyActor.generated.h"

UCLASS()
class DRAGONBALLSZ_API AEnemyActor : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyActor();

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="Enemy")
	void OnSightDetect(bool Target);

public:
	virtual void Tick(float DeltaTime) override;

public: // Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy")
	class UStatSystem* StatSystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy")
	class USightSystem* SightSystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy")
	class UEnemyFSM* EnemyFSM;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy")
	class UEnemyAnimInstance* AnimBP;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy")
	class AEnemyAIController* AIEnemy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy")
	class APlayerActor* TargetActor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy")
	bool IsHit = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy")
	float RotateLerpSpeed = 5.0f;
};



