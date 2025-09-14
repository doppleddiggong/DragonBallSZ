// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#pragma once

#include "CoreMinimal.h"
#include "ACombatCharacter.h"
#include "IControllable.h"
#include "APlayerActor.generated.h"

UCLASS()
class DRAGONBALLSZ_API APlayerActor : public ACombatCharacter, public IControllable
{
	GENERATED_BODY()

public:
	APlayerActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void Landed(const FHitResult& Hit) override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy")
	class UCameraShakeSystem* CameraShakeSystem;

public: // Control Interface
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Command")
	void Cmd_Move(const FVector2D& Axis) override;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Command")
	void Cmd_Look(const FVector2D& Axis) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Command")
	void Cmd_Jump() override;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Command")
	void Cmd_Dash() override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Command")
	void Cmd_Landing() override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Command")
	void Cmd_ChargeKi(bool bPressed) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Command")
	void Cmd_Guard(bool bPressed) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Command")
	void Cmd_Vanish() override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Command")
	void Cmd_RushAttack() override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Command")
	void Cmd_EnergyBlast() override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Command")
	void Cmd_Kamehameha() override;

public:
	UFUNCTION(BlueprintCallable, Category="Avoid")
	void OnRestoreAvoid();
	UFUNCTION(BlueprintCallable, Category="Event")
	void OnDash(AActor* Target, bool IsDashing, FVector Direction);
	UFUNCTION(BlueprintCallable, Category="Event")
	void OnTeleport(AActor* Target);
	UFUNCTION(BlueprintCallable, Category="Event")
	void OnAttack(AActor* Target, int ComboCount);
	UFUNCTION(BlueprintCallable, Category="Event")
	void OnSpecialAttack(AActor* Target, int32 SpecialIndex);
	UFUNCTION(BlueprintCallable, Category="Event")
	void OnGuard(AActor* Target, bool bState);
	UFUNCTION(BlueprintCallable, Category="Event")
	void OnAvoid(AActor* Target, bool bState);
    UFUNCTION(BlueprintCallable, Category="Event")
    void OnPowerCharge(AActor* Target, bool bState);

public:
	// 에너지탄 발사 딜레이
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EnergyBlastFactory")
	float BlastShotDelay = 0.5;

	// 에너지탄 최대 발사 연속 발사
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="EnergyBlastFactory")
	int MaxRepeatBlastShot = 5;
	
	// 재충전 딜레이
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="EnergyBlastFactory")
	float BlastShotRechargeDuration = 5.0;

	// 현재 남은 잔탄
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="EnergyBlastFactory")
	int RemainBlastShot  = MaxRepeatBlastShot;

	// 마지막 발사 시간
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="EnergyBlastFactory")
	float LastBlastShotTime = 0;

	// 재충전 시간
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="EnergyBlastFactory")
	float BlastShotRechargeTime = 0;

private:
	UPROPERTY()
	class UDBSZEventManager* EventManager;
};