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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

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

private:
	UPROPERTY()
	class UDBSZEventManager* EventManager;
};