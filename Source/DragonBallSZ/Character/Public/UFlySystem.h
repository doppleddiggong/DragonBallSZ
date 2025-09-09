// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UFlySystem.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DRAGONBALLSZ_API UFlySystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UFlySystem();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category="Fly")
	FORCEINLINE void InitSystem(class AActor* InOwner) {
		this->Owner = InOwner;
	}

	UFUNCTION(BlueprintCallable, Category="Fly")
	void ActivateFlyProcess();
	UFUNCTION(BlueprintCallable, Category="Fly")
	void ActivateLandingProcess();

private:
	void FlyTick(float DeltaTime);
	void LandingTick(float DeltaTime);
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Fly")
	bool FlyingProcess = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fly")
	float FlyDuration = 1.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fly")
	float FlyHeight = 3000.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Fly")
	bool LandingProcess = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fly")
	float LadningDuration = 1.5f;
	
private:
	UPROPERTY();
	class AActor* Owner;

	float ElapsedTime = 0;

	FVector StartLocation;
	FVector EndLocation;
};
