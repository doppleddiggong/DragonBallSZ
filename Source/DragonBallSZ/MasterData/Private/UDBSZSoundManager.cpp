// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UDBSZSoundManager.h"
#include "USoundData.h"
#include "Kismet/GameplayStatics.h"

#define SOUND_DATA_PATH TEXT("/Game/CustomContents/MasterData/Sound_Data.Sound_Data")

void UDBSZSoundManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FSoftObjectPath AssetPath(SOUND_DATA_PATH);
	USoundData* LoadedAsset = Cast<USoundData>(StaticLoadObject(USoundData::StaticClass(), nullptr, *AssetPath.ToString()));

	if (LoadedAsset)
	{
		for (const auto& Pair : LoadedAsset->SoundData)
		{
			SoundAsset = LoadedAsset;
			SoundData = SoundAsset->SoundData;
			
			TSoftObjectPtr<USoundBase> SoundAssetPtr = Pair.Value;
			if (!SoundAssetPtr.IsNull())
			{
				SoundData.Add(Pair.Key, SoundAssetPtr.LoadSynchronous());
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load SoundDataAsset from path"));
	}
}

void UDBSZSoundManager::PlaySound(const ESoundType Type, const FVector Location)
{
	if (TObjectPtr<USoundBase>* FoundSound = SoundData.Find(Type))
	{
		if (*FoundSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), *FoundSound, Location);
		}
	}
}


void UDBSZSoundManager::PlaySound2D(const ESoundType Type)
{
	if ( TObjectPtr<USoundBase>* FoundSound = SoundData.Find(Type))
	{
		if ( *FoundSound )
		{
			UGameplayStatics::PlaySound2D(GetWorld(), *FoundSound);
		}
	}
}