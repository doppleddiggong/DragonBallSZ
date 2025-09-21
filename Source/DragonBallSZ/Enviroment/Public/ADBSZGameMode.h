// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "ECharacterType.h"
#include "GameFramework/GameModeBase.h"
#include "ADBSZGameMode.generated.h"

UCLASS()
class DRAGONBALLSZ_API ADBSZGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	UFUNCTION( BlueprintCallable, Category="Character")
	void SelectRandomPlayer();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	ECharacterType PlayerType = ECharacterType::Songoku;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	ECharacterType EnemyType = ECharacterType::Vegeta;
};

