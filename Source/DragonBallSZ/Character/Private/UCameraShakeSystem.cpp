// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UCameraShakeSystem.h"
#include "ACombatCharacter.h"
#include "FPadFeedbackData.h"
#include "UDBSZEventManager.h"

#define SMALL_CAMERASHAKE_PATH      TEXT("/Game/CustomContents/Blueprints/CameraShake/BP_SmallCameraShake.BP_SmallCameraShake_C")
#define NORMAL_CAMERASHAKE_PATH     TEXT("/Game/CustomContents/Blueprints/CameraShake/BP_NormalCameraShake.BP_NormalCameraShake_C")
#define LARGE_CAMERASHAKE_PATH      TEXT("/Game/CustomContents/Blueprints/CameraShake/BP_LargeCameraShake.BP_LargeCameraShake_C")
#define HUGE_CAMERASHAKE_PATH		TEXT("/Game/CustomContents/Blueprints/CameraShake/BP_HugeCameraShake.BP_HugeCameraShake_C")

UCameraShakeSystem::UCameraShakeSystem()
{
	PrimaryComponentTick.bCanEverTick = true;

	{
		static ConstructorHelpers::FClassFinder<UCameraShakeBase> TempCameraShake(SMALL_CAMERASHAKE_PATH);
		if ( TempCameraShake.Succeeded())
			CameraShakeMap.Add( EAttackPowerType::Small, TempCameraShake.Class);
	}

	{
		static ConstructorHelpers::FClassFinder<UCameraShakeBase> TempCameraShake(NORMAL_CAMERASHAKE_PATH);
		if ( TempCameraShake.Succeeded())
			CameraShakeMap.Add( EAttackPowerType::Normal, TempCameraShake.Class);
	}

	{
		static ConstructorHelpers::FClassFinder<UCameraShakeBase> TempCameraShake(LARGE_CAMERASHAKE_PATH);
		if ( TempCameraShake.Succeeded())
			CameraShakeMap.Add( EAttackPowerType::Large, TempCameraShake.Class);
	}

	{
		static ConstructorHelpers::FClassFinder<UCameraShakeBase> TempCameraShake(HUGE_CAMERASHAKE_PATH);
		if ( TempCameraShake.Succeeded())
			CameraShakeMap.Add( EAttackPowerType::Huge, TempCameraShake.Class);
	}
}

void UCameraShakeSystem::InitSystem(ACombatCharacter* InOwner)
{
	this->Owner = InOwner;

	if ( auto EventManager = UDBSZEventManager::Get(GetWorld()))
	{
		EventManager->OnCameraShake.AddDynamic(this, &UCameraShakeSystem::OnCameraShake);
		EventManager->OnForceCameraShake.AddDynamic(this, &UCameraShakeSystem::OnForceCameraShake);
	}
}


void UCameraShakeSystem::OnCameraShake(AActor* Target, EAttackPowerType Type)
{
	if (Target != Owner)
		return;

	// 카메라 쉐이크 재생
	OnForceCameraShake(Type);
}

void UCameraShakeSystem::OnForceCameraShake(EAttackPowerType Type)
{
	if ( auto PlayerController = GetWorld()->GetFirstPlayerController() )
	{
		auto FeedBackData = GetPadFeedbackData(Type);
		
		PlayerController->PlayerCameraManager->StartCameraShake(CameraShakeMap[Type]);
		PlayerController->PlayDynamicForceFeedback(FeedBackData.Amplitude, FeedBackData.Duration,
			true, true, true, true);
	}
}
