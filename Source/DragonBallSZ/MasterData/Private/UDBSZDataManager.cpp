// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UDBSZDataManager.h"
#include "Shared/FComponentHelper.h"
#include "DragonBallSZ.h"

#define HITSTOP_PATH TEXT("/Game/CustomContents/MasterData/MasterData_HitStop.MasterData_HitStop")


UDBSZDataManager::UDBSZDataManager()
{
    HitStopTable = FComponentHelper::LoadAsset<UDataTable>(HITSTOP_PATH);
}

void UDBSZDataManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    ReloadMasterData();
}

void UDBSZDataManager::Deinitialize()
{
    HitStopCache.Reset();
    bLoadHitStop = false;

    Super::Deinitialize();
}


void UDBSZDataManager::ReloadMasterData()
{
    LoadData_HitStopTable();
}


void UDBSZDataManager::LoadData_HitStopTable()
{
    HitStopCache.Reset();
    bLoadHitStop = false;

    UDataTable* TableObj = HitStopTable.LoadSynchronous();
    if (!TableObj)
    {
        UE_LOG(LogTemp, Error, TEXT("HitStopTable load failed: %s"), *HitStopTable.ToString());
        return;
    }

    static const FString ContextString(TEXT("HitStopTable"));
    for (const FName& RowName : TableObj->GetRowNames() )
    {
        if (const FHitStopData* Row = TableObj->FindRow<FHitStopData>(RowName, ContextString, true))
        {
            HitStopCache.Add(Row->Type, *Row); // 바로 캐싱
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