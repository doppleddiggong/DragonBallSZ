// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(DBSZ, Log, All);

#define CALLINFO (FString(__FUNCTION__) + TEXT("(") + FString::FromInt(__LINE__) + TEXT(")"))
#define PRINTINFO() UE_LOG(DBSZ, Warning, TEXT("%s"), *CALLINFO)
#define PRINTLOG(fmt, ...) UE_LOG(DBSZ, Warning, TEXT("%s : %s"), *CALLINFO, *FString::Printf(fmt, ##__VA_ARGS__))
