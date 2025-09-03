// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "UDBSZEventManager.h"

void UDBSZEventManager::SendMessage(const FString& InMsg)
{
	OnMessage.Broadcast(InMsg);
}