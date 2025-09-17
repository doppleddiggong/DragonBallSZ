// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UCombatUI.generated.h"

UCLASS()
class DRAGONBALLSZ_API UCombatUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	UFUNCTION(BlueprintCallable, Category="Time")
	void StartCombatTime();

	UFUNCTION(BlueprintCallable, Category="Time")
	void ClearCombatTime();
	
	UFUNCTION()
	void OnReceiveMessage(FString Msg);
	
protected:
	UFUNCTION()
	void OnRecvUpdateHealth(bool bIsPlayer, float CurHP, float MaxHP);

	UFUNCTION()
	void OnRecvUpdateKi(bool bIsPlayer, float CurKi, float MaxKi);
	
private:
	void UpdateTimer();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Time")
	float CombatTime = 0.0f;
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_RemainTime;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> ProgressBar_Player;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> ProgressBar_Enemy;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> ProgressBar_Player_Ki;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> ProgressBar_Enemy_Ki;

	
private:
	FTimerHandle CombatTimerHandle;
};
