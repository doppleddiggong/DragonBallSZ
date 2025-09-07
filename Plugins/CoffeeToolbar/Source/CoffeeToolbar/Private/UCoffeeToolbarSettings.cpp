// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UCoffeeToolbarSettings.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"

UCoffeeToolbarSettings::UCoffeeToolbarSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	FDirectoryPath P;
	P.Path = TEXT("Game/CustomContents/Levels");
	ExtraSearchPaths.Add(P);
}

static bool NormalizeUserPath(const FString& In, FName& OutRoot)
{
	FString S = In;
	S.ReplaceInline(TEXT("\\"), TEXT("/"));
	S.TrimStartAndEndInline();

	if (S.Contains(TEXT(":")))
		return false;
	
	if (!S.StartsWith(TEXT("/")))
		S = TEXT("/") + S;

	if (S.StartsWith(TEXT("/Content/")))
		S = S.Replace(TEXT("/Content/"), TEXT("/Game/"));

	S.RemoveFromEnd(TEXT("/"));
	OutRoot = FName(*S);
	return true;
}

TArray<FName> UCoffeeToolbarSettings::GetSearchRoots(const bool bFallbackToGame)
{
	TArray<FName> Paths;
	bool bAdded = false;

	if (const UCoffeeToolbarSettings* S = GetDefault<UCoffeeToolbarSettings>())
	{
		for (const FDirectoryPath& Dir : S->ExtraSearchPaths)
		{
			if (Dir.Path.IsEmpty())
				continue;

			FName Root;
			if (NormalizeUserPath(Dir.Path, Root))
			{
				Paths.AddUnique(Root);
				bAdded = true;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid search path (cannot normalize): %s"), *Dir.Path);
			}
		}
	}

	if (!bAdded && bFallbackToGame)
		Paths.Add(FName(TEXT("/Game")));

	FAssetRegistryModule& ARM = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AR = ARM.Get();

	for (int32 i = Paths.Num() - 1; i >= 0; --i)
	{
		if (!AR.PathExists(Paths[i]))
		{
			UE_LOG(LogTemp, Warning, TEXT("Skip invalid search path: %s"), *Paths[i].ToString());
			Paths.RemoveAt(i);
		}
	}

	return Paths;
}