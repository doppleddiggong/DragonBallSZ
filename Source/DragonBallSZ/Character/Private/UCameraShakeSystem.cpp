// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UCameraShakeSystem.h"

#include "UDBSZEventManager.h"
#include "ACombatCharacter.h"

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
		EventManager->OnCameraShake.AddDynamic(this, &UCameraShakeSystem::OnCameraShake);
}

void UCameraShakeSystem::OnCameraShake(AActor* Target, EAttackPowerType Type)
{
	if (Target != Owner)
		return;

	// 카메라 쉐이크 재생
	auto PlayerController = GetWorld()->GetFirstPlayerController();
	PlayerController->PlayerCameraManager->StartCameraShake(CameraShakeMap[Type]);
}