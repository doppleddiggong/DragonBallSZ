// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "USightSystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSightDetect, bool, Target);


UCLASS( Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DRAGONBALLSZ_API USightSystem : public UActorComponent
{
	GENERATED_BODY()

public:	
	USightSystem();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void InitSightSystem(class AActor* TargetActor, const float SightLength, const float SightAngle);
	
private:
	void SightLineTrace(const float InLength, const float InAngle) const;
	bool DetectSightLength(const float InLength) const;
	bool DetectSightAngle(const float InAngle) const;

	void AddElapsedTime();	
	float LerpAlpha() const;
	
public:
	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnSightDetect OnSightDetect;
	
private:
	UPROPERTY()
	class AActor* OwnerActor;
	
	UPROPERTY()
	class AActor* TargetActor;

	bool DetectTarget = false;
	float SightLength = 1000.0f;
	float SightAngle = 45.0f;
	float Duration = 1.0f;
	float ElapsedTime = 0.0f;
};
