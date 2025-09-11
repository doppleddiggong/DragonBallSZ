// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ADynamicCameraActor.generated.h"

UCLASS()
class DRAGONBALLSZ_API ADynamicCameraActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADynamicCameraActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(EditAnywhere)
	class USpringArmComponent* SpringArmComp;
	UPROPERTY(EditAnywhere)
	class UCameraComponent* CameraComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class ACharacter* PlayerRef;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class AActor* TargetRef;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CameraDistance = 300;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TargetDistance = 500;
	float CurrentDistance;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeadZonePlayer_X_Min = 0.2f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeadZonePlayer_X_Max = 0.8f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeadZonePlayer_Y_Min = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeadZonePlayer_Y_Max = 0.9f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeadZoneTarget_X_Min = 0.3f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeadZoneTarget_X_Max = 0.7f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeadZoneTarget_Y_Min = 0.2f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeadZoneTarget_Y_Max = 0.8f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CameraResetTime = 5;

public:
	UFUNCTION(BlueprintCallable)
	void PlayerRotationLock();

	UFUNCTION(BlueprintCallable)
	void CloseCameraRotation(float DeltaTime);

	UFUNCTION(BlueprintCallable)
	void ResetCameraLocation(float DeltaTime);

	UFUNCTION()
	void ResetCameraForwardLocation(float DeltaTime);

	UFUNCTION(BlueprintCallable)
	void ResetCameraRotation(float DeltaTime);

	bool TargetDeadZoneCheck(const AActor &Target);
};
