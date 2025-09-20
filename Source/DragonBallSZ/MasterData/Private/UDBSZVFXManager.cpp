// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UDBSZVFXManager.h"

#include "EAttackPowerType.h"
#include "DragonBallSZ.h"
#include "UVFXDataAsset.h"
#include "NiagaraFunctionLibrary.h"
#include "Shared/FComponentHelper.h"

#define VFX_DATA_PATH TEXT("/Game/CustomContents/MasterData/VFX_Data.VFX_Data")

UDBSZVFXManager::UDBSZVFXManager()
{
	if (auto LoadedAsset = FComponentHelper::LoadAsset<UVFXDataAsset>(VFX_DATA_PATH))
	{
		VFXAsset = LoadedAsset;
		VFXDataMap = VFXAsset->VFXData;
	}
}

void UDBSZVFXManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// FSoftObjectPath AssetPath(VFX_DATA_PATH);
	// UVFXDataAsset* LoadedAsset = Cast<UVFXDataAsset>(StaticLoadObject(UVFXDataAsset::StaticClass(), nullptr, *AssetPath.ToString()));
	//
	// if (LoadedAsset)
	// {
	// 	VFXAsset = LoadedAsset;
	// 	VFXDataMap = VFXAsset->VFXData;
	// }
	// else
	if (!VFXAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load VFXDataAsset from path"));
	}
}

EVFXType UDBSZVFXManager::ConvertType(EAttackPowerType Type)
{
	if( Type == EAttackPowerType::Small )
		return EVFXType::Hit_Small;
	else if( Type == EAttackPowerType::Normal )
		return EVFXType::Hit_Normal;
	else if( Type == EAttackPowerType::Large )
		return EVFXType::Hit_Large;
	else if( Type == EAttackPowerType::Huge )
		return EVFXType::Hit_Huge;

	return EVFXType::Hit_Small;
}

bool UDBSZVFXManager::GetVFXData(EVFXType Type, FVFXData& Out) const
{
	if (const FVFXData* Found = VFXDataMap.Find(Type))
	{
		Out = *Found;
		return true;
	}

	PRINTLOG(TEXT("DataGetFail : %s"), *UEnum::GetValueAsString(Type) );
	return false;
}

void UDBSZVFXManager::ShowVFXAttackType( EAttackPowerType Type,
	FVector Location, FRotator Rotator, FVector Scale)
{
	auto ConvertVFX= ConvertType(Type);
	this->ShowVFX(ConvertVFX, Location, Rotator, Scale);
}

void UDBSZVFXManager::ShowVFX( EVFXType Type,
	FVector Location, FRotator Rotator, FVector Scale)
{
	FVFXData Params;
	if (!this->GetVFXData(Type, Params))
		return;

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		Params.VFXAsset,
		Location + Params.LocationOffset,
		Rotator + Params.RotatorOffset,
		Scale * Params.Scale,
		true,
		true,
		ENCPoolMethod::None,
		true
	);
}