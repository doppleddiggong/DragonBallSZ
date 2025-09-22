// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UCombatUI.h"

#include "GameEvent.h"
#include "ADBSZGameMode.h"
#include "UDBSZEventManager.h"

#include "TimerManager.h"

#include "Kismet/GameplayStatics.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/RichTextBlock.h"

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

	TextPlayerDamage->SetText(FText::GetEmpty());
	TextPlayerCombo->SetText(FText::GetEmpty());
	TextEnemyDamage->SetText(FText::GetEmpty());
	TextEnemyCombo->SetText(FText::GetEmpty());

	LeftHideDelegate.BindDynamic(this, &UCombatUI::OnLeftHideAnimFinished);
	RightHideDelegate.BindDynamic(this, &UCombatUI::OnRightHideAnimFinished);

	BindToAnimationFinished(LeftHideAnimation, LeftHideDelegate);
	BindToAnimationFinished(RightHideAnimation, RightHideDelegate);
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

void UCombatUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdatePlayerDamageUI();
	UpdateEnemyDamageUI();
}

void UCombatUI::UpdatePlayerDamageUI()
{
	IntPlayerDamageSum = FMath::RoundToInt(PlayerDamageSum);
	if (IntPlayerDamageSum == 0)
	{
		PlayerDamage = 0;
		return;
	}

	if (PlayerDamage > IntPlayerDamageSum) return;

	PlayerDamage += (IntPlayerDamageSum - PlayerDamage) * SpeedFactor;
	if (PlayerDamage <= IntPlayerDamageSum)
	{
		FString DamageStr = FString::FromInt(PlayerDamage);
		FString FormattedStr = DamageStr.Left(1) + "<Small>" + DamageStr.RightChop(1) + "</>";
		TextPlayerDamage->SetText(FText::FromString(FormattedStr));
	}
	else
	{
		FString DamageStr = FString::FromInt(PlayerDamage);
		FString FormattedStr = DamageStr.Left(1) + "<Small>" + DamageStr.RightChop(1) + "</>";
		TextPlayerDamage->SetText(FText::FromString(FormattedStr));
		
	}
}

void UCombatUI::UpdateEnemyDamageUI()
{
	IntEnemyDamageSum = FMath::RoundToInt(EnemyDamageSum);
	if (IntEnemyDamageSum == 0)
	{
		EnemyDamage = 0;
		return;
	}

	if (EnemyDamage > IntEnemyDamageSum) return;

	EnemyDamage += (IntEnemyDamageSum - EnemyDamage) * SpeedFactor;
	if (EnemyDamage <= IntEnemyDamageSum)
	{
		FString DamageStr = FString::FromInt(EnemyDamage);
		FString FormattedStr = DamageStr.Left(1) + "<Small>" + DamageStr.RightChop(1) + "</>";
		TextEnemyDamage->SetText(FText::FromString(FormattedStr));
		
	}
	else
	{
		FString DamageStr = FString::FromInt(EnemyDamage);
		FString FormattedStr = DamageStr.Left(1) + "<Small>" + DamageStr.RightChop(1) + "</>";
		TextEnemyDamage->SetText(FText::FromString(FormattedStr));
		
	}
}

void UCombatUI::UpdateTimer()
{
	CombatTime++;
	if (TextRemainTime)
	{
		const int32 DisplayTime = FMath::Max(0, FMath::FloorToInt(CombatTime));
		TextRemainTime->SetText(FText::AsNumber(DisplayTime));
	}
}

void UCombatUI::OnRecvUpdateHealth(bool bIsPlayer, float CurHP, float MaxHP)
{
	if (MaxHP <= 0.0f)
		return;

	const float Percent = FMath::Clamp(CurHP / MaxHP, 0.0f, 1.0f);

	if (bIsPlayer)
	{
		this->PlayerCurHP = CurHP;
		this->PlayerMaxHP = MaxHP;

		if (ProgressBar_Player)
			ProgressBar_Player->SetPercent(Percent);
	}
	else
	{
		this->EnemyCurHP = CurHP;
		this->EnemyMaxHP = MaxHP;

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
		this->PlayerCurKi = CurKi;
		this->PlayerMaxKi = MaxKi;

		if (ProgressBar_Player_Ki)
			ProgressBar_Player_Ki->SetPercent(Percent);
	}
	else
	{
		this->EnemyCurKi = CurKi;
		this->EnemyMaxKi = MaxKi;

		if (ProgressBar_Enemy_Ki)
			ProgressBar_Enemy_Ki->SetPercent(Percent);
	}
}

void UCombatUI::StartCombat(
	const float PlayerHP, const float EnemyHP,
	const float PlayerKi, const float EnemyKi)
{
	PlayerMaxHP = PlayerHP;
	PlayerCurHP = PlayerMaxHP;

	PlayerMaxKi = PlayerKi;
	PlayerCurKi = PlayerMaxKi;

	EnemyMaxHP = EnemyHP;
	EnemyCurHP = EnemyMaxHP;

	EnemyMaxKi = EnemyKi;
	EnemyCurKi = EnemyMaxKi;

	TextPlayerDamage->SetText(FText::GetEmpty());
	TextPlayerCombo->SetText(FText::GetEmpty());
	TextEnemyDamage->SetText(FText::GetEmpty());
	TextEnemyCombo->SetText(FText::GetEmpty());

	ProgressBar_Player->SetPercent(PlayerCurHP / PlayerMaxHP);
	ProgressBar_Enemy->SetPercent(EnemyCurHP / EnemyMaxHP);

	ProgressBar_Player_Ki->SetPercent(PlayerCurKi / PlayerMaxKi);
	ProgressBar_Enemy_Ki->SetPercent(EnemyCurKi / EnemyMaxKi);

	StartCombatTime();

	auto GameMode = Cast<ADBSZGameMode>(UGameplayStatics::GetGameMode(this));
	this->UpdateFace(GameMode->PlayerType, GameMode->EnemyType);
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
	if (Msg == GameEvent::PlayerWin || Msg == GameEvent::EnemyWin)
	{
		this->ClearCombatTime();
	}
}

void UCombatUI::OnDamage(bool bIsPlayer, float Damage)
{
	if (Damage <= 0)
		return;

	if (bIsPlayer)
	{
		// 플레이어가 맞았으니, 적에게 점수
		OnEnemyAttackHit(Damage);

		// 맞았으니 초기화
		PlayerDamageSum = 0;
		PlayerComboCount = 0;
		HidePlayerDamageUI();
	}
	else
	{
		// 적이 맞았으니, 플레이어에게 점수
		OnPlayerAttackHit(Damage);

		// 맞았으니 초기화
		EnemyDamageSum = 0;
		EnemyComboCount = 0;
		HideEnemyDamageUI();
	}
}

void UCombatUI::OnPlayerAttackHit(float Damage)
{
	// 플레이어 콤보 및 데미지 합산
	PlayerDamageSum += Damage;
	PlayerComboCount++;

	// 플레이어 UI 텍스트 업데이트
	if (TextPlayerDamage && TextPlayerCombo)
	{
		TextPlayerCombo->SetText(
			FText::Format(FText::FromString(TEXT("{0}<Small> Hits</>")), FText::AsNumber(PlayerComboCount)));

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
{
	if (bIsPlayerDamageUIVisible || !LeftShowAnimation)
	{
		return;
	}

	if (IsAnimationPlaying(LeftHideAnimation))
	{
		StopAnimation(LeftHideAnimation);
	}
	
	PlayAnimation(LeftShowAnimation);
	bIsPlayerDamageUIVisible = true;
}

void UCombatUI::HidePlayerDamageUI()
{
	if (!bIsPlayerDamageUIVisible || !LeftHideAnimation)
	{
		return;
	}

	if (IsAnimationPlaying(LeftShowAnimation))
	{
		StopAnimation(LeftShowAnimation);
	}

	if (!IsAnimationPlaying(LeftHideAnimation))
	{
		PlayAnimation(LeftHideAnimation);
	}
}


void UCombatUI::OnEnemyAttackHit(float Damage)
{
	// 적 콤보 및 데미지 합산
	EnemyDamageSum += Damage;
	EnemyComboCount++;

	// 적 UI 텍스트 업데이트
	if (TextEnemyDamage && TextEnemyCombo)
	{
		TextEnemyCombo->SetText(FText::Format(FText::FromString(TEXT("{0}<Small> Hits</>")),
		                                      FText::AsNumber(EnemyComboCount)));
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
{
	if (bIsEnemyDamageUIVisible || !RightShowAnimation)
	{
		return;
	}

	if (IsAnimationPlaying(RightHideAnimation))
	{
		StopAnimation(RightHideAnimation);
	}
	
	PlayAnimation(RightShowAnimation);
	bIsEnemyDamageUIVisible = true;
}

void UCombatUI::HideEnemyDamageUI()
{
	if (!bIsEnemyDamageUIVisible || !RightHideAnimation)
	{
		return;
	}

	if (IsAnimationPlaying(RightShowAnimation))
	{
		StopAnimation(RightShowAnimation);
	}

	if (!IsAnimationPlaying(RightHideAnimation))
	{
		PlayAnimation(RightHideAnimation);
	}
}

void UCombatUI::OnLeftHideAnimFinished()
{
	TextPlayerDamage->SetText(FText::GetEmpty());
	TextPlayerCombo->SetText(FText::GetEmpty());
	LeftComboImage->SetVisibility(ESlateVisibility::Hidden);
	bIsPlayerDamageUIVisible = false;
}

void UCombatUI::OnRightHideAnimFinished()
{
	TextEnemyDamage->SetText(FText::GetEmpty());
	TextEnemyCombo->SetText(FText::GetEmpty());
	RightComboImage->SetVisibility(ESlateVisibility::Hidden);
	bIsEnemyDamageUIVisible = false;
}
