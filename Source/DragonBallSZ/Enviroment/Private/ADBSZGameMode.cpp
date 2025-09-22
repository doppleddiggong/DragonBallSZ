// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "ADBSZGameMode.h"

void ADBSZGameMode::SelectRandomPlayer()
{
	int CharacterCount = (int)ECharacterType::MAX;

	PlayerType = static_cast<ECharacterType>(FMath::RandRange(0, CharacterCount - 1));
	bIsPlayerAnother = false;

	EnemyType = static_cast<ECharacterType>(FMath::RandRange(0, CharacterCount - 1));
	bIsEnemyAnother = PlayerType == EnemyType;
}

void ADBSZGameMode::StartGameWithCharacters(
	const ECharacterType NewPlayerType, const bool bNewPlayerAnother,
	const ECharacterType NewEnemyType, const bool bNewEnemyAnother)
{
	PlayerType = NewPlayerType;
	bIsPlayerAnother = bNewPlayerAnother;
	EnemyType = NewEnemyType;
	bIsEnemyAnother = bNewEnemyAnother;

	UE_LOG(LogTemp, Warning, TEXT("ADBSZGameMode::StartGameWithCharacters Called. Player: %d, Enemy: %d"), static_cast<int32>(PlayerType), static_cast<int32>(EnemyType));
}
