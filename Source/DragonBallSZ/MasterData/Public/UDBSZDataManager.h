// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Core/Macro.h"
#include "FHitStopData.h"
#include "FKnockbackData.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UDBSZDataManager.generated.h"

UCLASS()
class DRAGONBALLSZ_API UDBSZDataManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	DEFINE_SUBSYSTEM_GETTER_INLINE(UDBSZDataManager);
	UDBSZDataManager();

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(Exec)
	void ReloadMasterData();
	
#pragma region HIT_STOP
public:
	UPROPERTY(EditAnywhere, Category="MasterData|Tables")
	TSoftObjectPtr<UDataTable> HitStopTable;

	bool GetHitStopData(EAttackPowerType Power, FHitStopData& Out) const;

private:
	void Clear_HitStopTable();
	void LoadData_HitStopTable();
	bool bLoadHitStop = false;
	TMap<EAttackPowerType, FHitStopData> HitStopCache;
#pragma endregion HIT_STOP


#pragma region KNOCKBACK
public:
	UPROPERTY(EditAnywhere, Category="MasterData|Tables")
	TSoftObjectPtr<UDataTable> KnockbackTable;

	bool GetKnockbackData(EAttackPowerType Power, FKnockbackData& Out) const;

private:
	void Clear_KnockbackTable();
	void LoadData_KnockbackTable();
	bool bLoadKnockback = false;
	TMap<EAttackPowerType, FKnockbackData> KnockbackCache;
#pragma endregion KNOCKBACK
};
