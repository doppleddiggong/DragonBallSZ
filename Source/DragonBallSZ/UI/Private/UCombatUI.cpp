// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UCombatUI.h"
#include "UDBSZEventManager.h"

#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

void UCombatUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (auto EventManager = UDBSZEventManager::Get(GetWorld()))
		EventManager->OnUpdateHealth.AddDynamic(this, &UCombatUI::OnRecvUpdateHealth);
}

void UCombatUI::UpdateTimer(float Time)
{
	if (Text_RemainTime)
	{
		const int32 DisplayTime = FMath::Max(0, FMath::FloorToInt(Time));
		Text_RemainTime->SetText(FText::AsNumber(DisplayTime));
	}
}

void UCombatUI::OnRecvUpdateHealth(bool bIsPlayer, float CurHP, float MaxHP)
{
	if (MaxHP <= 0.0f)
		return;

	const float Percent = FMath::Clamp(CurHP / MaxHP, 0.0f, 1.0f);

	if (bIsPlayer)
	{
		if (ProgressBar_Player)
			ProgressBar_Player->SetPercent(Percent);
	}
	else
	{
		if (ProgressBar_Enemy)
			ProgressBar_Enemy->SetPercent(Percent);
	}
}
