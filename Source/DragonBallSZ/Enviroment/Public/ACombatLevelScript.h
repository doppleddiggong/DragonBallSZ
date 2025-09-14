// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Engine/LevelScriptActor.h"
#include "ACombatLevelScript.generated.h"

UCLASS()
class DRAGONBALLSZ_API ACombatLevelScript : public ALevelScriptActor
{
	GENERATED_BODY()

public:
	ACombatLevelScript();
	
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRecvMessage(FString InMsg);

private:
	void GameStart();
	void OnTimerTick();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GameState")
	float CombatStartDelay = 0.5f;

	
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<class UUserWidget> CombatUIFactory;
	UPROPERTY()
	class UCombatUI* CombatUI;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category="UI")
	float CombatTime = 0.0f;
};
