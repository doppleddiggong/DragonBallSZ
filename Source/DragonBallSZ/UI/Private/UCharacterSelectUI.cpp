// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UCharacterSelectUI.h"
#include "Components/Button.h"
#include "ASelectCamera.h"
#include "Kismet/GameplayStatics.h"

void UCharacterSelectUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (LeftButton)
		LeftButton->OnClicked.AddDynamic(this, &UCharacterSelectUI::OnLeftButtonClicked);
	if (RightButton)
		RightButton->OnClicked.AddDynamic(this, &UCharacterSelectUI::OnRightButtonClicked);
	if (SelectButton)
		SelectButton->OnClicked.AddDynamic(this, &UCharacterSelectUI::OnSelectButtonClicked);
	if (CancelSelectionButton)
		CancelSelectionButton->OnClicked.AddDynamic(this, &UCharacterSelectUI::OnCancelSelectionButtonClicked);
	if (GameStartButton)
		GameStartButton->OnClicked.AddDynamic(this, &UCharacterSelectUI::OnGameStartButtonClicked);

	AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), ASelectCamera::StaticClass());
	if (FoundActor)
		SelectCamera = Cast<ASelectCamera>(FoundActor);

	UpdateUI();
}

void UCharacterSelectUI::UpdateUI()
{
	if (!SelectCamera)
	{
		return;
	}

	const ESelectionState FocusState = SelectCamera->GetCurrentFocusSelectionState();

	SelectButton->SetVisibility(FocusState == ESelectionState::None ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	CancelSelectionButton->SetVisibility(FocusState != ESelectionState::None ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	const bool bCanGameStart = SelectCamera->IsPlayerSelected() && SelectCamera->IsEnemySelected();
	GameStartButton->SetVisibility(bCanGameStart ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UCharacterSelectUI::OnLeftButtonClicked()
{
	if (SelectCamera)
	{
		SelectCamera->MoveCharacter(-1);
		UpdateUI();
	}
}

void UCharacterSelectUI::OnRightButtonClicked()
{
	if (SelectCamera)
	{
		SelectCamera->MoveCharacter(1);
		UpdateUI();
	}
}

void UCharacterSelectUI::OnSelectButtonClicked()
{
	if (SelectCamera)
	{
		SelectCamera->SelectCurrentCharacter();
		UpdateUI();
	}
}

void UCharacterSelectUI::OnCancelSelectionButtonClicked()
{
	if (SelectCamera)
	{
		SelectCamera->DeselectCharacter();
		UpdateUI();
	}
}

void UCharacterSelectUI::OnGameStartButtonClicked()
{
}
