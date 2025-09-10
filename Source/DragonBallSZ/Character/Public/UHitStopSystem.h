// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EAttackPowerType.h"
#include "UHitStopSystem.generated.h"

USTRUCT(BlueprintType)
struct FHitStopParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)	float TimeDilation = 0.05f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	float Duration = 0.10f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	bool bRefreshIfStronger = true;

	static FHitStopParams GetParamsFromType(EAttackPowerType PowerType)
	{
		FHitStopParams RetParams;
		switch (PowerType)
		{
		case EAttackPowerType::Small:
			RetParams.TimeDilation = 0.1f;
			RetParams.Duration     = 0.1f;
			break;
			
		case EAttackPowerType::Normal:
			RetParams.TimeDilation = 0.06f;
			RetParams.Duration     = 0.15f;
			break;
			
		case EAttackPowerType::Large:
			RetParams.TimeDilation = 0.04f;
			RetParams.Duration     = 0.22f;
			break;
			
		case EAttackPowerType::Huge:
			RetParams.TimeDilation = 0.02f;
			RetParams.Duration     = 0.30f;
			break;

		default:
			break;
		}
		
		return RetParams;
	}
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
	void OnHitStopIssued(AActor* Target, const EAttackPowerType Type);

	UFUNCTION(BlueprintCallable, Category="HitStop")
	void InitSystem(ACharacter* InOwner);

	UFUNCTION(BlueprintCallable, Category="HitStop")
	void ApplyHitStop(const EAttackPowerType Type);


	
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
