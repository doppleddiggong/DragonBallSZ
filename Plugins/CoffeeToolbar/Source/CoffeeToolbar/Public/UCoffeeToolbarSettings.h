// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UCoffeeToolbarSettings.generated.h"

UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="CoffeeToolbar"))
class COFFEETOOLBAR_API UCoffeeToolbarSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UCoffeeToolbarSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }
	virtual FName GetSectionName()  const override { return TEXT("Level Selector"); }

	static TArray<FName> GetSearchRoots(const bool bFallbackToGame = true);

	
	UPROPERTY(EditAnywhere, Config, Category="Search")
	TArray<FDirectoryPath> ExtraSearchPaths;
};
