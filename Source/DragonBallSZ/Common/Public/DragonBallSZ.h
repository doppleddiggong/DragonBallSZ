// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(DBSZ, Log, All);

#define CALLINFO (FString(__FUNCTION__) + TEXT("(") + FString::FromInt(__LINE__) + TEXT(")"))
#define PRINTINFO() UE_LOG(DBSZ, Warning, TEXT("%s"), *CALLINFO)
#define PRINTLOG(fmt, ...) UE_LOG(DBSZ, Warning, TEXT("%s : %s"), *CALLINFO, *FString::Printf(fmt, ##__VA_ARGS__))

#define PRINT_STRING(fmt, ...) \
([&](){ \
const FString __msg__ = FString::Printf(fmt, ##__VA_ARGS__); \
UE_LOG(DBSZ, Warning, TEXT("%s : %s"), *CALLINFO, *__msg__); \
if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Cyan, __msg__); } \
}())
