// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "Framework/Commands/Commands.h"
#include "FCoffeeToolbarStyle.h"

class FCoffeeToolbarCommands : public TCommands<FCoffeeToolbarCommands>
{
public:
	FCoffeeToolbarCommands()
		: TCommands<FCoffeeToolbarCommands>(
			TEXT("CoffeeToolbar"),
			NSLOCTEXT("Contexts", "CoffeeToolbar", "CoffeeToolbar Plugin"),
			NAME_None,
			FCoffeeToolbarStyle::GetStyleSetName())
	{
	}

	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> PluginAction;
};
