// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "FCoffeeToolbarCommands.h"

#define LOCTEXT_NAMESPACE "FCoffeeToolbarModule"

void FCoffeeToolbarCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "CoffeeToolbar", "Execute CoffeeToolbar action", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
