#pragma once

#include "CoreMinimal.h"
#include "ECharacterType.h"
#include "Engine/DataTable.h"
#include "FCharacterAssetData.generated.h"

USTRUCT(BlueprintType)
struct FCharacterAssetData: public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Asset)
	ECharacterType CharacterType = ECharacterType::Songoku;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Asset)
	TSoftObjectPtr<class UCharacterData> CharacterDataAsset;
};