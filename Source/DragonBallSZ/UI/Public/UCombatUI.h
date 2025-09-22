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
	void StartCombat(
	const float PlayerHP, const float EnemyHP,
	const float PlayerKi, const float EnemyKi);
	
	UFUNCTION(BlueprintImplementableEvent, Category="Character")
	void UpdateFace(ECharacterType PlayerType, ECharacterType EnemyType);
	
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	void UpdatePlayerDamageUI();
	int32 PlayerDamage;
	int32 IntPlayerDamageSum;
	void UpdateEnemyDamageUI();
	int32 EnemyDamage;
	int32 IntEnemyDamageSum;
	float SpeedFactor = 0.1f;
	
	FWidgetAnimationDynamicEvent LeftHideDelegate;
	FWidgetAnimationDynamicEvent RightHideDelegate;
	UFUNCTION()
	void OnLeftHideAnimFinished();
	UFUNCTION()
	void OnRightHideAnimFinished();
	
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
	TObjectPtr<class URichTextBlock> TextRemainTime;

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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player")
	float PlayerCurHP;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player")
	float PlayerMaxHP;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player")
	float PlayerCurKi;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player")
	float PlayerMaxKi;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player")
	float PlayerDamageSum = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player")
	int32 PlayerComboCount = 0;

	// 적 체력
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	float EnemyCurHP;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	float EnemyMaxHP;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	float EnemyCurKi;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	float EnemyMaxKi;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	float EnemyDamageSum = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	int32 EnemyComboCount = 0;

	
private:
	bool bIsPlayerDamageUIVisible = false;
	bool bIsEnemyDamageUIVisible = false;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class URichTextBlock> TextPlayerDamage;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class URichTextBlock> TextPlayerCombo;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> LeftComboImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class URichTextBlock> TextEnemyDamage;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class URichTextBlock> TextEnemyCombo;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> RightComboImage;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* LeftShowAnimation;
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* RightShowAnimation;
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* LeftHideAnimation;
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* RightHideAnimation;

	
	FTimerHandle PlayerComboResetTimerHandle;
	FTimerHandle PlayerUIDisplayTimerHandle;

	// 적 관련 변수
	FTimerHandle EnemyComboResetTimerHandle;
	FTimerHandle EnemyUIDisplayTimerHandle;

	// 블루프린트에서 설정할 수 있는 시간 변수들
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	float ComboResetTime = 2.0f;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	float UIDisplayTime = 3.0f;
#pragma endregion DAMAGE_COMBO
};
