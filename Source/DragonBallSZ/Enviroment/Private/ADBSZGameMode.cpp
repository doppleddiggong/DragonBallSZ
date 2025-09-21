// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "ADBSZGameMode.h"

void ADBSZGameMode::SelectRandomPlayer()
{
	int CharacterCount = 2;
	PlayerType = static_cast<ECharacterType>(FMath::RandRange(0, CharacterCount - 1));
	EnemyType = static_cast<ECharacterType>(FMath::RandRange(0, CharacterCount - 1));
}