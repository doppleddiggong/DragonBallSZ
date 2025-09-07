// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "AEnemyAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

AEnemyAIController::AEnemyAIController()
{
	// 필요시 기본값 설정
	bAttachToPawn = true;
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// 블랙보드 초기화 + BT 실행
	if (BehaviorTreeAsset && BehaviorTreeAsset->BlackboardAsset)
	{
		UBlackboardComponent* BB = nullptr;
		const bool bOK = UseBlackboard(BehaviorTreeAsset->BlackboardAsset, BB);
		if (bOK && BB)
		{
			RunBehaviorTree(BehaviorTreeAsset);
		}
	}
}

void AEnemyAIController::SetTarget(AActor* Player)
{
	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->SetValueAsObject(PlayerKeyName, Player);  // BP의 Set Value as Object와 동일
	}
}