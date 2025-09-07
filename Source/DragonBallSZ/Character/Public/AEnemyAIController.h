// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "AEnemyAIController.generated.h"

UCLASS()
class DRAGONBALLSZ_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyAIController();

public:
	virtual void OnPossess(APawn* InPawn) override;

	UFUNCTION(BlueprintCallable, Category="AI")
	void SetTarget(AActor* Player);               // BP의 "Set Target" 이벤트 대응

	UPROPERTY(EditDefaultsOnly, Category="AI")
	UBehaviorTree* BehaviorTreeAsset = nullptr;   // 사용 중인 BT

	UPROPERTY(EditDefaultsOnly, Category="AI")
	FName PlayerKeyName = TEXT("Player");         // BP의 Make Literal Name "Player"
};