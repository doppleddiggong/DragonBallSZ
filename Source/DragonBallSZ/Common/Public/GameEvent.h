#pragma once

namespace GameEvent
{
	static FName GameStart = FName(TEXT("GameStart"));
	static FName CombatStart = FName(TEXT("CombatStart"));
	
	static FName PlayerWin = FName(TEXT("PlayerWin"));
	static FName EnemyWin = FName(TEXT("EnemyWin"));
}