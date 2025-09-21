// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Core/Macro.h"
#include "FHitStopData.h"
#include "FKnockbackData.h"
#include "FCharacterInfoData.h"
#include "FCharacterAssetData.h"

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
	UPROPERTY(EditAnywhere, Category="MasterData|HitStop")
	TSoftObjectPtr<UDataTable> HitStopTable;

    UFUNCTION(BlueprintCallable, Category="MasterData|HitStop")
    bool GetHitStopData(EAttackPowerType Power, UPARAM(ref) FHitStopData& Out) const;
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="MasterData|HitStop")
	float GetHitStopDelayTime(EAttackPowerType Type) const;

private:
	void Clear_HitStopTable();
	void LoadData_HitStopTable();
	bool bLoadHitStop = false;

	UPROPERTY(VisibleAnywhere, Category = "Cache", meta = (AllowPrivateAccess = "true"))
	TMap<EAttackPowerType, FHitStopData> HitStopCache;
#pragma endregion HIT_STOP

#pragma region KNOCKBACK
public:
	UPROPERTY(EditAnywhere, Category="MasterData|Knockback")
	TSoftObjectPtr<UDataTable> KnockbackTable;

	UFUNCTION(BlueprintCallable, Category="MasterData|Knockback")
	bool GetKnockbackData(EAttackPowerType Power, FKnockbackData& Out) const;

private:
	void Clear_KnockbackTable();
	void LoadData_KnockbackTable();
	bool bLoadKnockback = false;

	UPROPERTY(VisibleAnywhere, Category = "Cache", meta = (AllowPrivateAccess = "true"))
	TMap<EAttackPowerType, FKnockbackData> KnockbackCache;
#pragma endregion KNOCKBACK

#pragma region CHARACTER_INFO_DATA
public:
	UPROPERTY(EditAnywhere, Category="MasterData|CharacterInfo")
	TSoftObjectPtr<UDataTable> CharacterInfoTable;

	UFUNCTION(BlueprintCallable, Category="MasterData|CharacterInfo")
	bool GetCharacterInfoData(ECharacterType Type, FCharacterInfoData& Out) const;

private:
	void Clear_CharacterInfoData();
	void LoadData_CharacterInfoData();
	bool bLoadCharacterInfo = false;

	UPROPERTY(VisibleAnywhere, Category = "Cache", meta = (AllowPrivateAccess = "true"))
	TMap<ECharacterType, FCharacterInfoData> CharacterInfoCache;
#pragma endregion CHARACTER_INFO_DATA


#pragma region CHARACTER_ASSET_DATA
public:
	UPROPERTY(EditAnywhere, Category="MasterData|CharacterAsset")
	TSoftObjectPtr<UDataTable> CharacterAssetTable;

	UFUNCTION(BlueprintCallable, Category="MasterData|CharacterAsset")
	bool GetCharacterAssetData(ECharacterType Type, FCharacterAssetData& Out) const;

private:
	void Clear_CharacterAssetData();
	void LoadData_CharacterAssetData();
	bool bLoadCharacterAsset = false;

	UPROPERTY(VisibleAnywhere, Category = "Cache", meta = (AllowPrivateAccess = "true"))
	TMap<ECharacterType, FCharacterAssetData> CharacterAssetCache;
#pragma endregion CHARACTER_ASSET_DATA
};
