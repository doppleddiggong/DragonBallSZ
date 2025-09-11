// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "IControllable.h"
#include "EBodyPartType.h"
#include "URushAttackSystem.h"
#include "APlayerActor.generated.h"

UCLASS()
class DRAGONBALLSZ_API APlayerActor : public ACharacter, public IControllable
{
	GENERATED_BODY()

public:
	APlayerActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void Landed(const FHitResult& Hit) override;

public: // Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class UStatSystem* StatSystem;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class UHitStopSystem* HitStopSystem;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class UKnockbackSystem* KnockbackSystem;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class URushAttackSystem* RushAttackSystem;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class UDashSystem* DashSystem;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class UFlySystem* FlySystem;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class UArrowComponent* LeftHandComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class UArrowComponent* RightHandComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class UArrowComponent* LeftFootComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class UArrowComponent* RightFootComp;

public:
	FORCEINLINE UArrowComponent* GetBodyPart(EBodyPartType Part) const
	{
		switch (Part)
		{
			case EBodyPartType::Hand_L: return LeftHandComp;
			case EBodyPartType::Hand_R: return RightHandComp;
			case EBodyPartType::Foot_L: return LeftFootComp;
			case EBodyPartType::Foot_R: return RightFootComp;
			default:	return LeftHandComp;
		}
	}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure, Category="Command")
	bool IsControlEnable();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure, Category="Command")
	bool IsMoveEnable();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure, Category="Command")
	bool IsAttackEnable();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure, Category="Command")
	bool IsAttackIng();
	UFUNCTION(BlueprintPure, Category="Player|Sight")
	bool IsInSight(const AActor* Other) const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="LookTarget")
	void OnLookTarget();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Fly")
	void OnFlyEnd();
	UFUNCTION(BlueprintCallable, Category="Avoid")
	void OnRestoreAvoid();
	UFUNCTION(BlueprintCallable, Category="Fly")
	EMovementMode SetFlying();
	
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

	
public: // Control Interface
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player")
	bool IsSprinting = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player")
	bool IsHit = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player")
	class AEnemyActor* TargetActor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Player|Dash")
	TObjectPtr<class UNiagaraSystem> DashNiagaraSystem = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player|Sight", meta=(ClampMin="0"))
    float SightRange = 1200.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player|Sight", meta=(ClampMin="0", ClampMax="180"))
    float SightHalfFOVDeg = 60.0f; // half-angle degrees

private:
	FTimerHandle AvoidTimer;
	float AvoidTime = 1.0f;
	
	UPROPERTY()
	class UDBSZEventManager* EventManager;
	
public:
	UFUNCTION(BlueprintCallable, Category="Event")
	void OnDash(AActor* Target, bool IsDashing);
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
};
