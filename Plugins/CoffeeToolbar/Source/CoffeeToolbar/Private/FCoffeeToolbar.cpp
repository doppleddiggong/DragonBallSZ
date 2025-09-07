// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "FCoffeeToolbar.h"
#include "FCoffeeToolbarStyle.h"
#include "FCoffeeToolbarCommands.h"
#include "UCoffeeToolbarSettings.h"

#include "ToolMenus.h"
#include "UnrealEdGlobals.h"
#include "FileHelpers.h"
#include "LevelEditor.h"
#include "IAssetViewport.h"

#include "Misc/MessageDialog.h"
#include "Editor/UnrealEdEngine.h"
#include "Modules/ModuleManager.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Camera/CameraActor.h"
#include "Styling/SlateIconFinder.h"

#define LOCTEXT_NAMESPACE "FDoppleToolbar"

static UWorld* GetActiveTargetWorld()
{
	if (GEditor && GEditor->PlayWorld)
		return GEditor->PlayWorld;
	return GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
}

void FCoffeeToolbar::StartupModule()
{
	FCoffeeToolbarStyle::Initialize();
	FCoffeeToolbarStyle::ReloadTextures();

	FCoffeeToolbarCommands::Register();

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FCoffeeToolbar::RegisterMenus));
}

void FCoffeeToolbar::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FCoffeeToolbarStyle::Shutdown();

	FCoffeeToolbarCommands::Unregister();
}

bool FCoffeeToolbar::IsStatFPSChecked() const
{
	return bStatFPSEnabled;
}

void FCoffeeToolbar::EnsureDefaultSelection()
{
	if (!SelectedMapPackage.IsEmpty())
		return;

	TArray<FAssetData> Worlds;
	if (!CollectWorlds(Worlds) || Worlds.Num() == 0)
		return;

	SelectedMapPackage = Worlds[0].PackageName.ToString();
}

bool FCoffeeToolbar::CollectWorlds(TArray<FAssetData>& OutWorlds) const
{
	const TArray<FName> Roots = UCoffeeToolbarSettings::GetSearchRoots();

	FAssetRegistryModule& ARM = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AR = ARM.Get();

#if WITH_EDITOR
	// 에셋 레지스트리 스캔이 끝날 때까지 대기(필요 시)
	AR.WaitForCompletion();
#endif

	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.ClassPaths.Add(UWorld::StaticClass()->GetClassPathName());
	for (const FName& Root : Roots)
		Filter.PackagePaths.Add(Root);

	AR.GetAssets(Filter, OutWorlds);

	OutWorlds.Sort([](const FAssetData& A, const FAssetData& B)
	{
		if (A.AssetName != B.AssetName)
			return A.AssetName.LexicalLess(B.AssetName);
		
		return A.PackageName.LexicalLess(B.PackageName);
	});

	return OutWorlds.Num() > 0;
}

bool FCoffeeToolbar::EnsureValidSelectedMap() const
{
	if (SelectedMapPackage.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("No Level Selected!"));
		return false;
	}
	if (!FPackageName::IsValidLongPackageName(SelectedMapPackage))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid long package name: %s"), *SelectedMapPackage);
		return false;
	}
	if (!FPackageName::DoesPackageExist(SelectedMapPackage))
	{
		UE_LOG(LogTemp, Warning, TEXT("Map does not exist: %s"), *SelectedMapPackage);
		return false;
	}
	return true;
}

bool FCoffeeToolbar::LoadSelectedMap() const
{
	if (!EnsureValidSelectedMap())
		return false;

	// 필요 시 저장 유도:
	FEditorFileUtils::SaveDirtyPackages(true, true, false, false);

	if (!UEditorLoadingAndSavingUtils::LoadMap(SelectedMapPackage))
	{
		UE_LOG(LogTemp, Warning, TEXT("Load failed: %s"), *SelectedMapPackage);
		return false;
	}
	return true;
}

void FCoffeeToolbar::RegisterMenus()
{
	EnsureDefaultSelection();
	
	FToolMenuOwnerScoped OwnerScoped(this);

	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar"); 
	FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
	{
		const TAttribute<FText> LabelAttr = TAttribute<FText>::CreateLambda([this]()
		{
			if (SelectedMapPackage.IsEmpty())
				return NSLOCTEXT("CoffeeToolbar", "Idle", "Level Selector");
			
			const FString AssetName = FPackageName::GetLongPackageAssetName(SelectedMapPackage);
			return FText::FromString(AssetName);
		});

		FToolMenuEntry DropdownEntry = FToolMenuEntry::InitComboButton(
			"CoffeeToolbarDropdown",
			FUIAction(),
			FOnGetContent::CreateRaw(this, &FCoffeeToolbar::GenerateLevelMenu),
			LabelAttr,
			LOCTEXT("CoffeeToolbarDropdown_Tooltip", "Select Level"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Level")
		);
		DropdownEntry.StyleNameOverride = "CalloutToolbar";
		Section.AddEntry(DropdownEntry);
	}

	Section.AddEntry(FToolMenuEntry::InitToolBarButton(
		"CoffeeToolbar_Open",
		FUIAction(FExecuteAction::CreateRaw(this, &FCoffeeToolbar::OnSelectedMap_Open)),
		NSLOCTEXT("CoffeeToolbar","Open","Open"),
		NSLOCTEXT("CoffeeToolbar","Open_Tip","Open The Selected Level"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "GraphEditor.Documentation_16x")
	));

	Section.AddEntry(FToolMenuEntry::InitToolBarButton(
		"CoffeeToolbar_PlaySelectedViewport",
		FUIAction(FExecuteAction::CreateRaw(this, &FCoffeeToolbar::OnSelectedMap_PlaySelectedViewport)),
		NSLOCTEXT("CoffeeToolbar","PlaySelectedViewport","Play Selected Viewport"),
		NSLOCTEXT("CoffeeToolbar","PlaySelectedViewport_Tip","Play In The Selected Editor Viewport"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Play")
	));

	Section.AddEntry(FToolMenuEntry::InitToolBarButton(
		"CoffeeToolbar_PlayPIE",
		FUIAction(FExecuteAction::CreateRaw(this, &FCoffeeToolbar::OnSelectedMap_PlayInEditor)),
		NSLOCTEXT("CoffeeToolbar","PlayPIE","Play In Editor"),
		NSLOCTEXT("CoffeeToolbar","PlayPIE_Tip","Selected Editor Play In Editor"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "GraphEditor.PadEvent_16x")
	));

	Section.AddEntry(FToolMenuEntry::InitToolBarButton(
		"CoffeeToolbar_Screenshot",
		FUIAction(FExecuteAction::CreateRaw(this, &FCoffeeToolbar::OnCaptureScreenshot)),
		NSLOCTEXT("CoffeeToolbar", "Screenshot", "Screenshot"),
		NSLOCTEXT("CoffeeToolbar", "Screenshot_Tip", "Take a screenshot of the active viewport (PIE or Editor)"),
		FSlateIconFinder::FindIconForClass(ACameraActor::StaticClass()) // ← 카메라 아이콘
	));

	Section.AddEntry(FToolMenuEntry::InitToolBarButton(
		"DoppleToolbar_OpenShotDir",
		FUIAction(FExecuteAction::CreateRaw(this, &FCoffeeToolbar::OnOpenScreenShotDir)),
		NSLOCTEXT("CoffeeToolbar", "OpenShotDir", "Open Screenshot Folder"),
		NSLOCTEXT("CoffeeToolbar", "OpenShotDir_Tip", "Open the project's screenshot directory"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.FolderOpen")
	));

	{
		FToolMenuEntry StatEntry = FToolMenuEntry::InitToolBarButton(
			"DoppleToolbar_ToggleStatFPS",
			FUIAction(
				FExecuteAction::CreateRaw(this, &FCoffeeToolbar::OnToggleStatFPS),
				FCanExecuteAction(),
				FIsActionChecked::CreateRaw(this, &FCoffeeToolbar::IsStatFPSChecked)
			),
			NSLOCTEXT("CoffeeToolbar", "StatFPS", "STAT FPS"),
			NSLOCTEXT("CoffeeToolbar", "StatFPS_Tip", "Toggle 'stat fps' on active viewport (PIE or Editor)"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.StatsViewer")
		);
		StatEntry.UserInterfaceActionType = EUserInterfaceActionType::ToggleButton;
		Section.AddEntry(StatEntry);
	}

	UToolMenus::Get()->RefreshAllWidgets();
}

TSharedRef<SWidget> FCoffeeToolbar::GenerateLevelMenu()
{
	const TArray<FName> Roots = UCoffeeToolbarSettings::GetSearchRoots();

	FAssetRegistryModule& ARM = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AR = ARM.Get();
	
	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.ClassPaths.Add(UWorld::StaticClass()->GetClassPathName());
	
	for (const FName& Root : Roots)
		Filter.PackagePaths.Add(Root);

	TArray<FAssetData> Worlds;
	AR.GetAssets(Filter, Worlds);

	TMap<FName, int32> NameCount;
	for (const FAssetData& AD : Worlds)
		NameCount.FindOrAdd(AD.AssetName)++;

	Worlds.Sort([](const FAssetData& A, const FAssetData& B)
	{
		if (A.AssetName != B.AssetName)
			return A.AssetName.LexicalLess(B.AssetName);
		
		return A.PackageName.LexicalLess(B.PackageName);
	});

	FMenuBuilder MenuBuilder(true, nullptr);

	for (const FAssetData& AssetData : Worlds)
	{
		const FString PackagePath = AssetData.PackageName.ToString();                    // "/Game/Level/DefaultMap"
		const FString NameOnly    = AssetData.AssetName.ToString();                      // "DefaultMap"
		const FString DirOnly     = FPackageName::GetLongPackagePath(PackagePath);		 // "/Game/Level"
		const bool   bDup         = NameCount[AssetData.AssetName] > 1;

		const FText Label   = bDup
			? FText::FromString(FString::Printf(TEXT("%s  (%s)"), *NameOnly, *DirOnly))
			: FText::FromString(NameOnly);
		const FText Tooltip = FText::FromString(PackagePath);

		MenuBuilder.AddMenuEntry(
			Label, Tooltip, FSlateIcon(),
			FUIAction(FExecuteAction::CreateRaw(this, &FCoffeeToolbar::OnSelectedMap, PackagePath))
		);
	}

	return MenuBuilder.MakeWidget();
}

void FCoffeeToolbar::OnSelectedMap(FString InLongPackageName)
{
	this->SelectedMapPackage = MoveTemp(InLongPackageName);
	UToolMenus::Get()->RefreshAllWidgets();
	
	UE_LOG(LogTemp, Log, TEXT("Selected Level: %s"), *InLongPackageName);
}

void FCoffeeToolbar::OnSelectedMap_Open()
{
	if (LoadSelectedMap())
		UE_LOG(LogTemp, Log, TEXT("Opened: %s"), *SelectedMapPackage);
}

void FCoffeeToolbar::OnSelectedMap_PlaySelectedViewport()
{
	if (!LoadSelectedMap())
		return;

	TSharedPtr<IAssetViewport> ActiveLevelViewport;
	if (FModuleManager::Get().IsModuleLoaded(TEXT("LevelEditor")))
	{
		auto& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
		ActiveLevelViewport = LevelEditorModule.GetFirstActiveViewport();
	}

	if (GUnrealEd)
	{
		FRequestPlaySessionParams Params;
		Params.WorldType = EPlaySessionWorldType::PlayInEditor;
		Params.SessionDestination = EPlaySessionDestinationType::InProcess;
		if (ActiveLevelViewport.IsValid())
			Params.DestinationSlateViewport = ActiveLevelViewport;
		GUnrealEd->RequestPlaySession(Params);
	}
}

void FCoffeeToolbar::OnSelectedMap_PlayInEditor()
{
	if (!EnsureValidSelectedMap())
		return;

	if (GUnrealEd)
	{
		FRequestPlaySessionParams Params;
		Params.WorldType = EPlaySessionWorldType::PlayInEditor;
		Params.SessionDestination = EPlaySessionDestinationType::InProcess;
		Params.GlobalMapOverride = SelectedMapPackage;
		GUnrealEd->RequestPlaySession(Params);
	}
}

void FCoffeeToolbar::OnCaptureScreenshot()
{
	// 1) PIE 중이면 게임뷰포트에 고해상도 캡쳐
	if (GEngine && GEditor &&
		GEditor->PlayWorld &&
		GEngine->GameViewport &&
		GEngine->GameViewport->Viewport)
	{
		FScreenshotRequest::RequestScreenshot(true);
		GEngine->GameViewport->Viewport->TakeHighResScreenShot();
		return;
	}

	// 2) 에디터 레벨 뷰포트가 있으면 그쪽에서 캡쳐
	if (FLevelEditorModule* LEM = FModuleManager::GetModulePtr<FLevelEditorModule>("LevelEditor"))
	{
		if (TSharedPtr<IAssetViewport> AV = LEM->GetFirstActiveViewport())
		{
			FEditorViewportClient& VC = AV->GetAssetViewportClient();
			VC.TakeHighResScreenShot();
			return;
		}
	}

	if (UWorld* W = GetActiveTargetWorld())
		GEditor->Exec(W, TEXT("HighResShot 1"));
}

void FCoffeeToolbar::OnOpenScreenShotDir()
{
	FString Dir = FPaths::ScreenShotDir();
	FString AbsoluteDir = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*Dir);
	IFileManager::Get().MakeDirectory(*AbsoluteDir, true);
	FPlatformProcess::ExploreFolder(*AbsoluteDir);
}

void FCoffeeToolbar::OnToggleStatFPS()
{
	bStatFPSEnabled = !bStatFPSEnabled;
	if (UWorld* W = GetActiveTargetWorld())
	{
		const TCHAR* Cmd = bStatFPSEnabled ? TEXT("stat fps 1") : TEXT("stat fps 0");
		GEditor->Exec(W, Cmd);
	}
}
#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCoffeeToolbar, CoffeeToolbar)