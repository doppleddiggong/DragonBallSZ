// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class ESelectionState : uint8
{
	None UMETA(DisplayName = "None"),
	PlayerSelected UMETA(DisplayName = "PlayerSelected"),
	EnemySelected UMETA(DisplayName = "EnemySelected"),
};
