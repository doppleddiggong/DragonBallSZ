// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "Modules/ModuleManager.h"

class FCoffeeToolbar : public IModuleInterface
{
public:
	bool IsStatFPSChecked() const;
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	TSharedRef<class SWidget> GenerateLevelMenu();
	
private:
	void RegisterMenus();

	void EnsureDefaultSelection();
	bool CollectWorlds(TArray<FAssetData>& OutWorlds) const;

	bool EnsureValidSelectedMap() const;
	bool LoadSelectedMap() const;
	
	void OnSelectedMap(FString InLongPackageName);
	void OnSelectedMap_Open();
	void OnSelectedMap_PlaySelectedViewport();
	void OnSelectedMap_PlayInEditor();

	void OnCaptureScreenshot();
	void OnOpenScreenShotDir();
	void OnToggleStatFPS();
	
	FString SelectedMapPackage;

	bool bStatFPSEnabled = false;
};