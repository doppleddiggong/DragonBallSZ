// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "FCoffeeToolbarStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedPtr<FSlateStyleSet> FCoffeeToolbarStyle::Instance = nullptr;

const ISlateStyle& FCoffeeToolbarStyle::Get()
{
	return *Instance;
}

void FCoffeeToolbarStyle::Initialize()
{
	if (!Instance.IsValid())
	{
		Instance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*Instance);
	}
}

void FCoffeeToolbarStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*Instance);
	ensure(Instance.IsUnique());
	Instance.Reset();
}

FName FCoffeeToolbarStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("FCoffeeToolbarStyle"));
	return StyleSetName;
}

TSharedRef<FSlateStyleSet> FCoffeeToolbarStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("CoffeeToolbarStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("CoffeeToolbar")->GetBaseDir() / TEXT("Resources"));

	Style->Set("CoffeeToolbar.PluginAction", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));
	return Style;
}

void FCoffeeToolbarStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}