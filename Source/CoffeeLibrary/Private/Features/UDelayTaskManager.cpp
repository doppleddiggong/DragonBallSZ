// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "Features/UDelayTaskManager.h"
#include "Engine/World.h"
#include "TimerManager.h"

#define CALLINFO (FString(__FUNCTION__) + TEXT("(") + FString::FromInt(__LINE__) + TEXT(")"))
#define PRINTLOG(fmt, ...) UE_LOG(LogTemp, Warning, TEXT("%s : %s"), *CALLINFO, *FString::Printf(fmt, ##__VA_ARGS__))

void UDelayTaskManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	// PRINTLOG( TEXT("DelayTaskManager Initialized."));
}

void UDelayTaskManager::Deinitialize()
{
    if (auto World = GetWorld())
    {
    	FTimerManager& TM = World->GetTimerManager();
    	
        for (const auto& KVP : Entries)
        {
            FTimerHandle TempHandle = KVP.Value.TimerHandle;
            World->GetTimerManager().ClearTimer(TempHandle);
        }
    }
    Entries.Empty();
    OwnerIndex.Empty();
    
	Super::Deinitialize();
}

FDelayHandle UDelayTaskManager::DelayCallBack(UObject* Owner, const float Seconds, FDelayCallback Callback)
{
    return Delay(Owner, Seconds, [Callback]() {
        if (Callback.IsBound())
            Callback.Execute();
    });
}

FDelayHandle UDelayTaskManager::Delay(UObject* Owner, const float Seconds, TFunction<void()> Action)
{
    FDelayHandle Out;
    if (!GetWorld() || Seconds < 0.f || !Action)
        return Out;

    const uint64 SeqIndex = NextSeqIndex++;

	FTimerHandle Handle;
	FTimerDelegate D;
	D.BindUObject(this, &UDelayTaskManager::ExecuteById, SeqIndex); // 핸들 대신 Seq 바인딩

	GetWorld()->GetTimerManager().SetTimer(Handle, D, Seconds, /*bLoop=*/false);

	FEntry& E  = Entries.Add(SeqIndex);
	E.Owner    = Owner;
	E.Action   = MoveTemp(Action);
	E.SeqIndex = SeqIndex;
	E.TimerHandle = Handle;

	if (Owner)
	{
		OwnerIndex.Add(Owner, SeqIndex);
	}

	Out.SeqIndex = SeqIndex;
	return Out;
}

FDelayHandle UDelayTaskManager::NextTick(UObject* Owner, TFunction<void()> Action)
{
    FDelayHandle Out;
    if (!GetWorld() || !Action)
        return Out;

    const uint64 SeqIndex = NextSeqIndex++;

	// 다음 틱 보장 & 취소 가능성을 위해 0초 타이머 사용(엔진은 다음 프레임에 실행)
	FTimerHandle Handle;
	FTimerDelegate D;
	D.BindUObject(this, &UDelayTaskManager::ExecuteById, SeqIndex);

	GetWorld()->GetTimerManager().SetTimer(Handle, D, /*Rate=*/0.0f, /*bLoop=*/false);
	
	FEntry& E  = Entries.Add(SeqIndex);
	E.Owner    = Owner;
	E.Action   = MoveTemp(Action);
	E.SeqIndex = SeqIndex;
	E.TimerHandle = Handle;

	if (Owner)
	{
		OwnerIndex.Add(Owner, SeqIndex);
	}
	
	Out.SeqIndex = SeqIndex;
	return Out;
}

bool UDelayTaskManager::Cancel(const FDelayHandle& Handle)
{
    if (!GetWorld() || !Handle.IsValid())
        return false;

	FEntry Entry;
	if (Entries.RemoveAndCopyValue(Handle.SeqIndex, Entry))
	{
		// 엔진 타이머도 정리
		GetWorld()->GetTimerManager().ClearTimer(Entry.TimerHandle);
		OwnerIndex.RemoveSingle(Entry.Owner, Handle.SeqIndex);
		return true;
	}

	return false;
}

void UDelayTaskManager::CancelAll(UObject* Owner)
{
    if (!GetWorld() || !Owner)
        return;

	TArray<uint64> Ids;
	OwnerIndex.MultiFind(Owner, Ids);
	if (Ids.Num() == 0)
		return;

	FTimerManager& TM = GetWorld()->GetTimerManager();

	for (uint64 Seq : Ids)
	{
		FEntry Entry;
		if (Entries.RemoveAndCopyValue(Seq, Entry))
		{
			TM.ClearTimer(Entry.TimerHandle);
			OwnerIndex.RemoveSingle(Entry.Owner, Seq);
		}
	}
}

void UDelayTaskManager::ExecuteById(uint64 SeqIndex)
{
	FEntry Entry;
	if (!Entries.RemoveAndCopyValue(SeqIndex, Entry))
	{
		return;
	}

	const bool bOwnerValid = Entry.Owner.IsValid();
	if (bOwnerValid  && Entry.Action )
	{
		Entry.Action();
	}

	OwnerIndex.RemoveSingle(Entry.Owner, SeqIndex);
}

void UDelayTaskManager::RemoveEntryById(uint64 SeqIndex)
{
	if (FEntry* Found = Entries.Find(SeqIndex))
	{
		OwnerIndex.RemoveSingle(Found->Owner, SeqIndex);
		Entries.Remove(SeqIndex);
	}
}