// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UHitStopSystem.generated.h"

USTRUCT(BlueprintType)
struct FHitStopParams
{
	GENERATED_BODY()

	// 0.0f ~ 1.0f (0.0에 가까울수록 더 얼어붙음. 0.01 추천)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeDilation = 0.05f;

	// 실시간 기준 지속(초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.10f;

	// 누적/갱신 정책
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRefreshIfStronger = true;
};


UCLASS( Blueprintable, ClassGroup=(DBSZ), meta=(BlueprintSpawnableComponent) )
class DRAGONBALLSZ_API UHitStopSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UHitStopSystem();

public:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	UFUNCTION()
	void OnHitStopIssued(AActor* Target, FHitStopParams Params);

	UFUNCTION(BlueprintCallable, Category="HitStop")
	void InitSystem(ACharacter* InOwner);

	UFUNCTION(BlueprintCallable, Category="HitStop")
	void ApplyHitStop(const FHitStopParams& Params);
	
private:
	void BeginFreeze(const FHitStopParams& Params);
	void EndFreeze();

private:
	UPROPERTY()
	class ACharacter* Owner;
	UPROPERTY()
	class USkeletalMeshComponent* MeshComp;
	UPROPERTY()
	class UCharacterMovementComponent* MoveComp;

	FHitStopParams LastParams;
	
	bool bActive = false;
	double EndRealTimeSeconds = 0.0;

	float SavedCustomTimeDilation = 1.0f;
};
