// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UFlySystem.generated.h"

DECLARE_DYNAMIC_DELEGATE(FEndCallback);

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
	FORCEINLINE void InitSystem(class ACharacter* InOwner, FEndCallback InCallback)
	{
		this->Owner = InOwner;
		this->Callback = InCallback;
	}

	UFUNCTION(BlueprintCallable, Category="Fly")
	void OnJump();
	UFUNCTION(BlueprintCallable, Category="Fly")
	void OnLand(const FHitResult& Hit);

	UFUNCTION(BlueprintCallable, Category="Fly")
	void ActivateUpstream();
	UFUNCTION(BlueprintCallable, Category="Fly")
	void ActivateDownstream();

private:
	void UpstreamTick(float DeltaTime);
	void DownstreamTick(float DeltaTime);
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Fly")
	bool bIsUpstream = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fly")
	float UpstreamDuration = 1.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fly")
	float UpstreamHeight = 3000.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Fly")
	bool bIsDownstream = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fly")
	float DownstreamDuration = 1.5f;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Fly")
	class ACharacter* Owner;
	
private:
	UPROPERTY()
	FEndCallback Callback;
	
	float ElapsedTime = 0;

	FVector StartLocation;
	FVector EndLocation;

	int JumpCount = 0;
};
