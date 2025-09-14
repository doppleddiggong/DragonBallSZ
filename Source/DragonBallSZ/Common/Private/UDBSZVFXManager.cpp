// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UDBSZVFXManager.h"
#include "NiagaraFunctionLibrary.h"
#include "UVFXData.h"

#define VFX_DATA_PATH TEXT("/Game/CustomContents/MasterData/VFX_Data.VFX_Data")

void UDBSZVFXManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FSoftObjectPath AssetPath(VFX_DATA_PATH);
	UVFXData* LoadedAsset = Cast<UVFXData>(StaticLoadObject(UVFXData::StaticClass(), nullptr, *AssetPath.ToString()));

	if (LoadedAsset)
	{
		VFXAsset = LoadedAsset;
		VFXData = VFXAsset->VFXData;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load VFXDataAsset from path"));
	}
}

void UDBSZVFXManager::ShowVFX( EVFXType Type, FVector Location, FRotator Rotator, FVector Scale)
{
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		VFXData[Type],
		Location,
		Rotator,
		Scale,
		true,
		true,
		ENCPoolMethod::None,
		true
	);
}
