// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Core/Macro.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UDBSZEventManager.generated.h"

UCLASS()
class DRAGONBALLSZ_API UDBSZEventManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	DEFINE_SUBSYSTEM_GETTER_INLINE(UDBSZEventManager);


	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMessage, FString, Msg);
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnMessage OnMessage;
	
	UFUNCTION(BlueprintCallable, Category="Events")
	void SendMessage(const FString& InMsg);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCamera, int32, Group, int32, Index);
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnCamera OnCamera;

	UFUNCTION(BlueprintCallable, Category="Events")
	void SendCamera(const int& Group, const int& Index);
};
