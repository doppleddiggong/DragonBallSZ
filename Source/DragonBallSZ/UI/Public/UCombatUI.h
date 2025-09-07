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

public:
	void UpdateTimer(float Time);

protected:
	UFUNCTION()
	void OnRecvUpdateHealth(bool bIsPlayer, float CurHP, float MaxHP);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_RemainTime;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> ProgressBar_Player;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> ProgressBar_Enemy;
};
