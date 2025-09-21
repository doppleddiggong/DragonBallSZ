// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UFlySystem.generated.h"

DECLARE_DYNAMIC_DELEGATE(FEndCallback);

UCLASS(ClassGroup=(DBSZ), meta=(BlueprintSpawnableComponent))
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
	void InitSystem(class ACombatCharacter* InOwner, FEndCallback InCallback);

	UFUNCTION(BlueprintCallable, Category="Fly")
	void OnAltitudePress(const bool bIsUp);
	UFUNCTION(BlueprintCallable, Category="Fly")
	void OnAltitudeRelease();

	
	UFUNCTION(BlueprintCallable, Category="Fly")
	void OnJump();
	UFUNCTION(BlueprintCallable, Category="Fly")
	void OnLand(const FHitResult& Hit);

	UFUNCTION(BlueprintCallable, Category="Fly")
	void ActivateUpstream();
    UFUNCTION(BlueprintCallable, Category="Fly")
    void ActivateDownstream();

private:
    void UpstreamTick(const float DeltaTime);
    void DownstreamTick(const float DeltaTime);
    FVector AltitudeTick(const float DeltaTime);

    // FVector HandleFlyDeceleration(float DeltaTime);
	
public:

	UPROPERTY(Transient, BlueprintReadOnly, Category="Fly")
	TObjectPtr<class ACombatCharacter> Owner;
	UPROPERTY(Transient, BlueprintReadOnly, Category="Fly")
	TObjectPtr<class UCharacterMovementComponent> MoveComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Fly")
	bool bIsUpstream = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fly")
	float UpstreamDuration = 1.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fly")
	float UpstreamHeight = 1500.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fly")
	float MaxFlightHeight = 3000.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Fly")
	bool bIsDownstream = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fly")
	float DownstreamDuration = 1.5f;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fly")
	float AlmostDist = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fly|Stat")	
	float CurrentFlySpeed = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fly|Stat")	
	float FlySpeedMin = 300.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fly|Stat")	
	float FlySpeedMax = 1500.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fly|Stat")	
	float Acceleration = 1000.0f; // per second
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fly|Stat")	
	bool bAltitudeInput = false;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fly|Stat")
    bool bIsAltitudeUp = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fly|Braking")
    float BrakingDecelerationZ = 2048.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fly|Braking")
    float BrakingFriction = 2.0f;

private:
	UPROPERTY()
	FEndCallback Callback;

	UPROPERTY()
	TObjectPtr<class UDBSZEventManager> EventManager;

	int JumpCount = 0;
	float ElapsedTime = 0;

	FVector StartLocation;
	FVector EndLocation;
};
