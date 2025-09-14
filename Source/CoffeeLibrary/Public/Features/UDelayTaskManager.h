// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Core/Macro.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Delegates/Delegate.h"
#include "UDelayTaskManager.generated.h"

USTRUCT(BlueprintType)
struct FDelayHandle
{
	GENERATED_BODY()

	uint64 SeqIndex = 0;

	bool IsValid() const
	{
		return SeqIndex != 0;
	}
};

DECLARE_DYNAMIC_DELEGATE(FDelayCallback);

UCLASS()
class COFFEELIBRARY_API UDelayTaskManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	DEFINE_SUBSYSTEM_GETTER_INLINE(UDelayTaskManager);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category="Sequence")
	FDelayHandle DelayCallBack(UObject* Owner, float Seconds, FDelayCallback Callback);

	// C++용 람다/함수버전
	FDelayHandle Delay(UObject* Owner, float Seconds, TFunction<void()> Action);
	// 다음 틱에 실행
	FDelayHandle NextTick(UObject* Owner, TFunction<void()> Action);

	UFUNCTION(BlueprintCallable, Category="Sequence")
	bool Cancel(const FDelayHandle& Handle);

	UFUNCTION(BlueprintCallable, Category="Sequence")
	void CancelAll(UObject* Owner);

private:
	// 타이머 만료 시 ID 기반 실행(핸들 전달 금지)
	UFUNCTION()
	void ExecuteById(uint64 SeqIndex);

	// 내부 제거 헬퍼
	void RemoveEntryById(uint64 SeqIndex);
private:
	struct FEntry
	{
		TWeakObjectPtr<UObject> Owner;
		TFunction<void()> Action;
		uint64 SeqIndex = 0;
		// 실제 엔진 핸들(로그/취소용)
		FTimerHandle TimerHandle;
	};

	// 키를 uint64 ID로 통일
	TMap<uint64, FEntry> Entries;
	
	// Owner -> 여러 ID
	TMultiMap<TWeakObjectPtr<UObject>, uint64> OwnerIndex;

	// ID 시퀀스
	uint64 NextSeqIndex = 1;
};