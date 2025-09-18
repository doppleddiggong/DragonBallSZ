// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UCombatUI.generated.h"

UCLASS()
class DRAGONBALLSZ_API UCombatUI : public UUserWidget
{
	GENERATED_BODY()

public:
	void StartCombat(const float PlayerKi, const float EnemyKi);
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	UFUNCTION()
	void OnReceiveMessage(FString Msg);

	UFUNCTION()
	void OnDamage(bool bIsPalyer, float Damage);

#pragma region COMBAT_TIME
public:
	UFUNCTION(BlueprintCallable, Category="Time")
	void StartCombatTime();

	UFUNCTION(BlueprintCallable, Category="Time")
	void ClearCombatTime();
	
private:
	void UpdateTimer();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Time")
	float CombatTime = 0.0f;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_RemainTime;

	FTimerHandle CombatTimerHandle;
#pragma endregion COMBAT_TIME

#pragma region UPATE_GAUGE
protected:
	UFUNCTION()
	void OnRecvUpdateHealth(bool bIsPlayer, float CurHP, float MaxHP);

	UFUNCTION()
	void OnRecvUpdateKi(bool bIsPlayer, float CurKi, float MaxKi);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> ProgressBar_Player;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> ProgressBar_Enemy;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> ProgressBar_Player_Ki;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> ProgressBar_Enemy_Ki;
#pragma endregion UPATE_GAUGE

#pragma region DAMAGE_COMBO
private:
	void OnPlayerAttackHit(float Damage);
	void ResetPlayerCombo();
	void ShowPlayerDamageUI();
	void HidePlayerDamageUI();

	void OnEnemyAttackHit(float Damage);
	void ResetEnemyCombo();
	void ShowEnemyDamageUI();
	void HideEnemyDamageUI();

public:
	// 플레이어 체력
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
	float PlayerCurHP;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
	float PlayerMaxHP;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
	float PlayerCurKi;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
	float PlayerMaxKi;

	
	// 적 체력
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
	float EnemyCurHP;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
	float EnemyMaxHP;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
	float EnemyCurKi;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
	float EnemyMaxKi;
	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_PlayerDamage;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_PlayerCombo;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_EnemyDamage;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_EnemyCombo;

	
	float PlayerDamageSum = 0.0f;
	int32 PlayerComboCount = 0;
	FTimerHandle PlayerComboResetTimerHandle;
	FTimerHandle PlayerUIDisplayTimerHandle;

	// 적 관련 변수
	float EnemyDamageSum = 0.0f;
	int32 EnemyComboCount = 0;
	FTimerHandle EnemyComboResetTimerHandle;
	FTimerHandle EnemyUIDisplayTimerHandle;

	// 블루프린트에서 설정할 수 있는 시간 변수들
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	float ComboResetTime = 2.0f;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	float UIDisplayTime = 3.0f;
#pragma endregion DAMAGE_COMBO
};
