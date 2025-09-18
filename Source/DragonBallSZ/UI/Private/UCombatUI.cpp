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
		EventManager->OnDamage.AddDynamic(this, &UCombatUI::OnDamage);
		EventManager->OnMessage.AddDynamic(this, &UCombatUI::OnReceiveMessage);
	}

	this->HidePlayerDamageUI();
	this->HideEnemyDamageUI();
}

void UCombatUI::NativeDestruct()
{
	Super::NativeDestruct();

	if (auto EventManager = UDBSZEventManager::Get(GetWorld()))
	{
		EventManager->OnUpdateHealth.RemoveDynamic(this, &UCombatUI::OnRecvUpdateHealth);
		EventManager->OnUpdateKi.RemoveDynamic(this, &UCombatUI::OnRecvUpdateKi);
		EventManager->OnDamage.RemoveDynamic(this, &UCombatUI::OnDamage);
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

	// this->HidePlayerDamageUI();
	// this->HideEnemyDamageUI();
	//
	// // TODO.. 이벤트의 순서의 문제인데...
	// ProgressBar_Player->SetPercent(1.0f);
	// ProgressBar_Enemy->SetPercent(1.0f);
	// ProgressBar_Player_Ki->SetPercent(0.5f);
	// ProgressBar_Enemy_Ki->SetPercent(0.5f);
	
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

void UCombatUI::OnDamage(bool bIsPlayer, float Damage)
{
	if( Damage <= 0 )
		return;
	
	if ( bIsPlayer )
	{
		// 플레이어가 맞았으니, 적에게 점수
		OnEnemyAttackHit(Damage);

		// 맞았으니 초기화
		PlayerDamageSum = 0;
		PlayerComboCount = 0;
		OnPlayerAttackHit(0);
	}
	else
	{
		// 적가 맞았으니, 플레이어에게 점수
		OnPlayerAttackHit(Damage);

		// 맞았으니 초기화
		EnemyDamageSum = 0;
		EnemyComboCount = 0;
		OnEnemyAttackHit(0);
	}
}

void UCombatUI::OnPlayerAttackHit(float Damage)
{
	// 플레이어 콤보 및 데미지 합산
	PlayerDamageSum += Damage;
	PlayerComboCount++;

	// 플레이어 UI 텍스트 업데이트
	if (Text_PlayerDamage && Text_PlayerCombo)
	{
		Text_PlayerDamage->SetText(FText::Format(FText::FromString(TEXT("{0} Damage")), FText::AsNumber(FMath::RoundToInt(PlayerDamageSum))));
		Text_PlayerCombo->SetText(FText::Format(FText::FromString(TEXT("{0} Combo")), FText::AsNumber(PlayerComboCount)));
		ShowPlayerDamageUI();
	}

	// 플레이어 콤보 리셋 타이머 시작/리셋
	GetWorld()->GetTimerManager().SetTimer(
		PlayerComboResetTimerHandle,
		this,
		&UCombatUI::ResetPlayerCombo,
		ComboResetTime,
		false
	);

	// 플레이어 UI 표시 타이머 시작/리셋
	GetWorld()->GetTimerManager().SetTimer(
		PlayerUIDisplayTimerHandle,
		this,
		&UCombatUI::HidePlayerDamageUI,
		UIDisplayTime,
		false
	);
}

void UCombatUI::ResetPlayerCombo()
{
	PlayerDamageSum = 0.0f;
	PlayerComboCount = 0;
}

void UCombatUI::ShowPlayerDamageUI()
{}

void UCombatUI::HidePlayerDamageUI()
{
	Text_PlayerDamage->SetText(FText::GetEmpty());
	Text_PlayerCombo->SetText(FText::GetEmpty());
}


void UCombatUI::OnEnemyAttackHit(float Damage)
{
	// 적 콤보 및 데미지 합산
	EnemyDamageSum += Damage;
	EnemyComboCount++;

	// 적 UI 텍스트 업데이트
	if (Text_EnemyDamage && Text_EnemyCombo)
	{
		Text_EnemyDamage->SetText(FText::Format(FText::FromString(TEXT("{0} Damage")), FText::AsNumber(FMath::RoundToInt(EnemyDamageSum))));
		Text_EnemyCombo->SetText(FText::Format(FText::FromString(TEXT("{0} Combo")), FText::AsNumber(EnemyComboCount)));
		ShowEnemyDamageUI();
	}

	// 적 콤보 리셋 타이머 시작/리셋
	GetWorld()->GetTimerManager().SetTimer(
		EnemyComboResetTimerHandle,
		this,
		&UCombatUI::ResetEnemyCombo,
		ComboResetTime,
		false
	);

	// 적 UI 표시 타이머 시작/리셋
	GetWorld()->GetTimerManager().SetTimer(
		EnemyUIDisplayTimerHandle,
		this,
		&UCombatUI::HideEnemyDamageUI,
		UIDisplayTime,
		false
	);
}

void UCombatUI::ResetEnemyCombo()
{
	EnemyDamageSum = 0.0f;
	EnemyComboCount = 0;
}

void UCombatUI::ShowEnemyDamageUI()
{}

void UCombatUI::HideEnemyDamageUI()
{
	Text_EnemyDamage->SetText(FText::GetEmpty());
	Text_EnemyCombo->SetText(FText::GetEmpty());
}