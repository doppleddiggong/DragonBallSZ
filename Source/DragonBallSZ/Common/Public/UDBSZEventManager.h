// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Core/Macro.h"
#include "UHitStopSystem.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UDBSZEventManager.generated.h"

UCLASS()
class DRAGONBALLSZ_API UDBSZEventManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	DEFINE_SUBSYSTEM_GETTER_INLINE(UDBSZEventManager);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMessage, FString, Msg);
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnMessage OnMessage;
	
	UFUNCTION(BlueprintCallable, Category="Events")
	void SendMessage(const FString& InMsg);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCamera, int32, Group, int32, Index);
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnCamera OnCamera;

	UFUNCTION(BlueprintCallable, Category="Events")
	void SendCamera(const int& Group, const int& Index);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnUpdateHealth, bool, IsPlayer, float, CurHP, float, MaxHp );
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnUpdateHealth OnUpdateHealth;

	UFUNCTION(BlueprintCallable, Category="Events")
	void SendUpdateHealth(const bool IsPlayer, const float CurHP, const float MaxHp);


	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHitStop, AActor*, Target, EAttackPowerType, Type);
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnHitStop OnHitStop;

	UFUNCTION(BlueprintCallable, Category="Events")
	void SendHitStop(AActor* Target, const EAttackPowerType Type);

	UFUNCTION(BlueprintCallable, Category="Events")
	void SendHitStopPair(AActor* Attacker, const EAttackPowerType AttackerType,
						 AActor* Target,   const EAttackPowerType TargetType);

	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnKnockback,
		AActor*,		   Target,
		AActor*,           Instigator,
		EAttackPowerType,  Type,
		float,             Resistance );
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnKnockback OnKnockback;

	UFUNCTION(BlueprintCallable, Category="Events")
	void SendKnockback(AActor* Target, AActor* Instigator, EAttackPowerType Type, float Resistance);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDash, AActor*, Target, bool, bIsDashing);
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnDash OnDash;

	UFUNCTION(BlueprintCallable, Category="Events")
	void SendDash(AActor* Target, bool bIsDashing);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTeleport, AActor*, Target);
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnTeleport OnTeleport;

	UFUNCTION(BlueprintCallable, Category="Events")
	void SendTeleport(AActor* Target);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttack, AActor*, Target, int, ComboCount );
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnAttack OnAttack;

	UFUNCTION(BlueprintCallable, Category="Events")
	void SendAttack(AActor* Target, int ComboCount);
};
