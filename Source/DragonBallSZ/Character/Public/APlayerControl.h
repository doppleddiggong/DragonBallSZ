// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "APlayerControl.generated.h"

UCLASS()
class DRAGONBALLSZ_API APlayerControl : public APlayerController
{
	GENERATED_BODY()
	
protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

	// --- Input Assets ---
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<class UInputMappingContext> IMC_Default;	

	UPROPERTY(EditDefaultsOnly, Category="Input") TObjectPtr<class UInputAction> IA_Move;
	UPROPERTY(EditDefaultsOnly, Category="Input") TObjectPtr<class UInputAction> IA_Look;

	UPROPERTY(EditDefaultsOnly, Category="Input") TObjectPtr<class UInputAction> IA_Jump;
	UPROPERTY(EditDefaultsOnly, Category="Input") TObjectPtr<class UInputAction> IA_Dash;

	UPROPERTY(EditDefaultsOnly, Category="Input") TObjectPtr<class UInputAction> IA_LockOn;

	UPROPERTY(EditDefaultsOnly, Category="Input") TObjectPtr<class UInputAction> IA_Guard;
	UPROPERTY(EditDefaultsOnly, Category="Input") TObjectPtr<class UInputAction> IA_Vanish;

	UPROPERTY(EditDefaultsOnly, Category="Input") TObjectPtr<class UInputAction> IA_RushAttack;
	UPROPERTY(EditDefaultsOnly, Category="Input") TObjectPtr<class UInputAction> IA_EnergyBlast;
	UPROPERTY(EditDefaultsOnly, Category="Input") TObjectPtr<class UInputAction> IA_ChargeKi;

	UPROPERTY(EditDefaultsOnly, Category="Input") TObjectPtr<class UInputAction> IA_Kamehameha;
	
	// --- Handlers ---
	void OnMove(const FInputActionValue& Value);
	void OnLook(const FInputActionValue& Value);

	void OnJump(const FInputActionValue& Value);
	void OnDash(const FInputActionValue& Value);

	void OnLockOn(const FInputActionValue& Value);
	
	void OnChargeKiPressed(const FInputActionValue& Value);
	void OnChargeKiReleased(const FInputActionValue& Value);
	void OnGuardPressed(const FInputActionValue& Value);
	void OnGuardReleased(const FInputActionValue& Value);
	void OnVanish(const FInputActionValue& Value);

	void OnRushAttack(const FInputActionValue& Value);
	void OnEnergyBlast(const FInputActionValue& Value);

	void OnKamehameha(const FInputActionValue& Value);
	
private:
    class IControllable* GetControllable() const;
};
