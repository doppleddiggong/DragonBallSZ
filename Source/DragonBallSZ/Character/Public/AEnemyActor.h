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
	UFUNCTION(BlueprintCallable, Category="Enemy")
	void OnSightDetect(bool Target);
	
public: // Enemy Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy")
	TObjectPtr<class USightSystem> SightSystem;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy")
	TObjectPtr<class UEnemyFSM> EnemyFSM;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy")
	float RotateLerpSpeed = 5.0f;
};