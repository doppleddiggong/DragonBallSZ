// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UCombatUI.h"

#include "GameEvent.h"
#include "UDBSZEventManager.h"

#include "TimerManager.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

void UCombatUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (auto EventManager = UDBSZEventManager::Get(GetWorld()))
	{
		EventManager->OnUpdateHealth.AddDynamic(this, &UCombatUI::OnRecvUpdateHealth);
		EventManager->OnUpdateKi.AddDynamic(this, &UCombatUI::OnRecvUpdateKi);
		EventManager->OnMessage.AddDynamic(this, &UCombatUI::OnReceiveMessage);
	}
}

void UCombatUI::NativeDestruct()
{
	Super::NativeDestruct();

	if (auto EventManager = UDBSZEventManager::Get(GetWorld()))
	{
		EventManager->OnUpdateHealth.RemoveDynamic(this, &UCombatUI::OnRecvUpdateHealth);
		EventManager->OnUpdateKi.RemoveDynamic(this, &UCombatUI::OnRecvUpdateKi);
		EventManager->OnMessage.RemoveDynamic(this, &UCombatUI::OnReceiveMessage);
	}

	GetWorld()->GetTimerManager().ClearTimer(CombatTimerHandle);
}

void UCombatUI::UpdateTimer()
{
	CombatTime++;
	if (Text_RemainTime)
	{
		const int32 DisplayTime = FMath::Max(0, FMath::FloorToInt(CombatTime));
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

void UCombatUI::OnRecvUpdateKi(bool bIsPlayer, float CurKi, float MaxKi)
{
	if (MaxKi <= 0.0f)
		return;

	const float Percent = FMath::Clamp(CurKi / MaxKi, 0.0f, 1.0f);

	if (bIsPlayer)
	{
		if (ProgressBar_Player_Ki)
			ProgressBar_Player_Ki->SetPercent(Percent);
	}
	else
	{
		if (ProgressBar_Enemy_Ki)
			ProgressBar_Enemy_Ki->SetPercent(Percent);
	}
}

void UCombatUI::StartCombatTime()
{
	CombatTime = 0.0f;

	UpdateTimer();
	GetWorld()->GetTimerManager().SetTimer(CombatTimerHandle, this, &UCombatUI::UpdateTimer, 1.0f, true);
}

void UCombatUI::ClearCombatTime()
{
	CombatTime = 0.0f;
	
	GetWorld()->GetTimerManager().ClearTimer(CombatTimerHandle);
}


void UCombatUI::OnReceiveMessage(FString Msg)
{
	if ( Msg == GameEvent::PlayerWin || Msg == GameEvent::EnemyWin )
	{
		this->ClearCombatTime();
	}
}