// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UCharacterSelectUI.generated.h"


UCLASS()
class DRAGONBALLSZ_API UCharacterSelectUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateUI();

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> LeftButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> RightButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> SelectButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> CancelSelectionButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> GameStartButton;

	UFUNCTION()
	void OnLeftButtonClicked();

	UFUNCTION()
	void OnRightButtonClicked();

	UFUNCTION()
	void OnSelectButtonClicked();

	UFUNCTION()
	void OnCancelSelectionButtonClicked();

	UFUNCTION()
	void OnGameStartButtonClicked();

	UPROPERTY()
	TObjectPtr<class ASelectCamera> SelectCamera;
};
