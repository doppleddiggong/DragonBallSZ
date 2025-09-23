// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UDBSZSoundManager.h"
#include "USoundData.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Shared/FComponentHelper.h"

#define SOUND_DATA_PATH TEXT("/Game/CustomContents/MasterData/Sound_Data.Sound_Data")


UDBSZSoundManager::UDBSZSoundManager()
{
	// FComponentHelper를 사용해 생성자에서 안전하게 에셋을 로드합니다.
	if (auto LoadedAsset = FComponentHelper::LoadAsset<USoundData>(SOUND_DATA_PATH))
	{
		for (const auto& Pair : LoadedAsset->SoundData)
		{
			SoundAsset = LoadedAsset;
			SoundData = SoundAsset->SoundData;
			
			TSoftObjectPtr<USoundBase> SoundAssetPtr = Pair.Value;
			if (!SoundAssetPtr.IsNull()) {
				SoundData.Add(Pair.Key, SoundAssetPtr.LoadSynchronous());
			}
		}
	}
	else
	{
		// 실패 시, 에디터 실행 시 Output Log에 에러를 출력합니다.
		UE_LOG(LogTemp, Error, TEXT("UDBSZSoundManager failed to load USoundData at path: %s"), SOUND_DATA_PATH);
	}
}

void UDBSZSoundManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// FSoftObjectPath AssetPath(SOUND_DATA_PATH);
	// USoundData* LoadedAsset = Cast<USoundData>(StaticLoadObject(USoundData::StaticClass(), nullptr, *AssetPath.ToString()));
	//
	// if (LoadedAsset)
	// {
	// 	for (const auto& Pair : LoadedAsset->SoundData)
	// 	{
	// 		SoundAsset = LoadedAsset;
	// 		SoundData = SoundAsset->SoundData;
	// 		
	// 		TSoftObjectPtr<USoundBase> SoundAssetPtr = Pair.Value;
	// 		if (!SoundAssetPtr.IsNull())
	// 		{
	// 			SoundData.Add(Pair.Key, SoundAssetPtr.LoadSynchronous());
	// 		}
	// 	}
	// }
	// else
	if (!SoundAsset)
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
	if (TObjectPtr<USoundBase>* FoundSound = SoundData.Find(Type))
	{
		if (*FoundSound)
		{
			// 이미 재생 중인 사운드가 있다면 멈추고 제거
			if (UAudioComponent* ExistingComp = ActiveSounds.FindRef(Type))
			{
				ExistingComp->Stop();
				ActiveSounds.Remove(Type);
			}

			// 새로운 사운드 재생 및 저장
			if (UAudioComponent* NewComp = UGameplayStatics::SpawnSound2D(GetWorld(), *FoundSound))
			{
				ActiveSounds.Add(Type, NewComp);
			}
		}
	}
}

void UDBSZSoundManager::StopSound2D(const ESoundType Type)
{
	if (UAudioComponent* Comp = ActiveSounds.FindRef(Type))
	{
		Comp->Stop();
		ActiveSounds.Remove(Type);
	}
}