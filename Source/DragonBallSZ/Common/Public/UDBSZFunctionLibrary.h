#pragma once

#include "CoreMinimal.h"
#include "UDBSZDamageType.h"
#include "EAttackPowerType.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UDBSZFunctionLibrary.generated.h"


UCLASS()
class DRAGONBALLSZ_API UDBSZFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="DBSZ|Damage")
	static TSubclassOf<UDBSZDamageType> GetDamageTypeClass(EAttackPowerType InType);
};
