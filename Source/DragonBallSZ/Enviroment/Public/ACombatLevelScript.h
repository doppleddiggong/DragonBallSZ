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

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UFUNCTION( BlueprintCallable, BlueprintPure, Category="NEW_STATE")
	FORCEINLINE class ADynamicCameraActor* GetDynamicCameraActor() const
	{
		return DynamicCameraActor;
	};

	UFUNCTION( BlueprintCallable, BlueprintPure, Category="NEW_STATE")
	FORCEINLINE class APlayerActor* GetPlayerActor() const
	{
		return PlayerActor;
	};

	UFUNCTION( BlueprintCallable, BlueprintPure, Category="NEW_STATE")
	FORCEINLINE class AEnemyActor* GetEnemyActor() const
	{
		return EnemyActor;
	};

	UFUNCTION()
	void OnRecvMessage(FString InMsg);
	UFUNCTION(BlueprintCallable, Category="NEW_STATE")
	void CombatResultProcess(bool IsPlayerWin);
	
public:
	UFUNCTION( BlueprintCallable, Category="UI")
	void ShowCombatUI(const ESlateVisibility InSetVisiblity );

	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<class UUserWidget> CombatUIFactory;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI" )
	class UCombatUI* CombatUI;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera" )
	class ADynamicCameraActor* DynamicCameraActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character" )
	class APlayerActor* PlayerActor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character" )
	class AEnemyActor* EnemyActor;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State")
	bool bCombatStart = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State")
	bool bCombatResult = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event" )
	class UDBSZEventManager* EventManager;

	
public:
	UFUNCTION( BlueprintCallable, Category="Sequence")
	void PlaySequence(class ULevelSequence* InSequence);

	UFUNCTION()
	void OnSequenceFinished();
	void PlaySoundWin();

	UPROPERTY()
	class ULevelSequence* PlayingSequence;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sequences")
	float CombatStartDelay = 2.0f;
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sequences")
	class ULevelSequencePlayer* SequencePlayer;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sequences")
	class ALevelSequenceActor* SequenceActor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sequences")
	class ULevelSequence* MainSeq;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sequences")
	class ULevelSequence* GokuWinSeq;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sequences")
	class ULevelSequence* VegeWinSeq;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sequences")
	AActor* GokuWinActor;
};
