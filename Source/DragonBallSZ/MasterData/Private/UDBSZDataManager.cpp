// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UDBSZDataManager.h"
#include "DragonBallSZ.h"
#include "Shared/FComponentHelper.h"

#define HITSTOP_PATH    TEXT("/Game/CustomContents/MasterData/HitStop.HitStop")
#define KNOCKBACK_PATH  TEXT("/Game/CustomContents/MasterData/Knockback.Knockback")
#define CHARACTERINFO_PATH  TEXT("/Game/CustomContents/MasterData/CharacterInfo.CharacterInfo")

UDBSZDataManager::UDBSZDataManager()
{
    HitStopTable = FComponentHelper::LoadAsset<UDataTable>(HITSTOP_PATH);
    KnockbackTable  = FComponentHelper::LoadAsset<UDataTable>(KNOCKBACK_PATH);
    CharacterInfoTable  = FComponentHelper::LoadAsset<UDataTable>(CHARACTERINFO_PATH);
}

void UDBSZDataManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    ReloadMasterData();
}

void UDBSZDataManager::Deinitialize()
{
    Clear_HitStopTable();
    Clear_KnockbackTable();
    Clear_CharacterInfoData();
    
    Super::Deinitialize();
}

void UDBSZDataManager::ReloadMasterData()
{
    LoadData_HitStopTable();
    LoadData_KnockbackTable();
    LoadData_CharacterInfoData();
}

#pragma region HIT_STOP
void UDBSZDataManager::Clear_HitStopTable()
{
    HitStopCache.Reset();
    bLoadHitStop = false;   
}

void UDBSZDataManager::LoadData_HitStopTable()
{
    HitStopCache.Reset();
    bLoadHitStop = false;

    UDataTable* TableObj = HitStopTable.LoadSynchronous();
    if (!TableObj)
    {
        PRINTLOG(TEXT("Load failed: %s"), *HitStopTable.ToString());
        return;
    }

    static const FString ContextString(TEXT("HitStopTable"));
    for (const FName& RowName : TableObj->GetRowNames() )
    {
        if (const FHitStopData* Row = TableObj->FindRow<FHitStopData>(RowName, ContextString, true))
        {
            HitStopCache.Add(Row->Type, *Row);
        }
    }

    bLoadHitStop = true;
}

bool UDBSZDataManager::GetHitStopData(EAttackPowerType Type, FHitStopData& Out) const
{
    if (!bLoadHitStop)
        return false;

    if (const FHitStopData* Found = HitStopCache.Find(Type))
    {
        Out = *Found;
        return true;
    }

    PRINTLOG(TEXT("DataGetFail : %s"), *UEnum::GetValueAsString(Type) );
    return false;
}

float UDBSZDataManager::GetHitStopDelayTime(EAttackPowerType Type) const
{
    if (!bLoadHitStop)
        return 0.0f;

    if (const FHitStopData* Found = HitStopCache.Find(Type))
        return Found->Duration;
    
    return 0.0f;
}

#pragma endregion HIT_STOP


#pragma region KNOCKBACK
void UDBSZDataManager::Clear_KnockbackTable()
{
    KnockbackCache.Reset();
    bLoadKnockback = false;   
}

void UDBSZDataManager::LoadData_KnockbackTable()
{
    KnockbackCache.Reset();
    bLoadKnockback = false;

    UDataTable* TableObj = KnockbackTable.LoadSynchronous();
    if (!TableObj)
    {
        PRINTLOG(TEXT("Load failed: %s"), *KnockbackTable.ToString());
        return;
    }

    static const FString ContextString(TEXT("KnockbackTable"));
    for (const FName& RowName : TableObj->GetRowNames() )
    {
        if (const FKnockbackData* Row = TableObj->FindRow<FKnockbackData>(RowName, ContextString, true))
        {
            KnockbackCache.Add(Row->Type, *Row);
        }
    }

    bLoadKnockback = true;
}

bool UDBSZDataManager::GetKnockbackData(EAttackPowerType Type, FKnockbackData& Out) const
{
    if (!bLoadKnockback)
        return false;

    if (const FKnockbackData* Found = KnockbackCache.Find(Type))
    {
        Out = *Found;
        return true;
    }

    PRINTLOG(TEXT("DataGetFail : %s"), *UEnum::GetValueAsString(Type) );
    return false;
}
#pragma endregion KNOCKBACK

#pragma region CHARACTER_DATA
void UDBSZDataManager::Clear_CharacterInfoData()
{
    CharacterInfoCache.Reset();
    bLoadCharacterInfo = false;   
}

void UDBSZDataManager::LoadData_CharacterInfoData()
{
    CharacterInfoCache.Reset();
    bLoadCharacterInfo = false;

    UDataTable* TableObj = CharacterInfoTable.LoadSynchronous();
    if (!TableObj)
    {
        PRINTLOG(TEXT("Load failed: %s"), *CharacterInfoTable.ToString());
        return;
    }

    static const FString ContextString(TEXT("CharacterInfoTable"));
    for (const FName& RowName : TableObj->GetRowNames() )
    {
        if (const FCharacterInfoData* Row = TableObj->FindRow<FCharacterInfoData>(RowName, ContextString, true))
        {
            CharacterInfoCache.Add(Row->Type, *Row);
        }
    }

    bLoadKnockback = true;
}

bool UDBSZDataManager::GetCharacterInfoData(ECharacterType Type, FCharacterInfoData& Out) const
{
    if (!bLoadCharacterInfo)
        return false;

    if (const FCharacterInfoData* Found = CharacterInfoCache.Find(Type))
    {
        Out = *Found;
        return true;
    }

    PRINTLOG(TEXT("DataGetFail : %s"), *UEnum::GetValueAsString(Type) );
    return false;
}
#pragma endregion CHARACTER_DATA