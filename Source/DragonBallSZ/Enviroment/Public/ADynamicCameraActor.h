// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ACombatCharacter.h"
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
	class ACombatCharacter* PlayerRef;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class ACombatCharacter* TargetRef;
	
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


public:// Event-Delegate
	UFUNCTION(BlueprintCallable, Category="Event")
	void OnDash(AActor* Target, bool IsDashing, FVector Direction);
	UFUNCTION(BlueprintCallable, Category="Event")
	void OnTeleport(AActor* Target);
	UFUNCTION(BlueprintCallable, Category="Event")
	void OnAttack(AActor* Target, int ComboCount);
	UFUNCTION(BlueprintCallable, Category="Event")
	void OnSpecialAttack(AActor* Target, int32 SpecialIndex);
	UFUNCTION(BlueprintCallable, Category="Event")
	void OnGuard(AActor* Target, bool bState);
	UFUNCTION(BlueprintCallable, Category="Event")
	void OnAvoid(AActor* Target, bool bState);
	UFUNCTION(BlueprintCallable, Category="Event")
	void OnPowerCharge(AActor* Target, bool bState);


	UFUNCTION(BlueprintCallable, Category="Command")
	FORCEINLINE void SetPlayerHold( bool bState)
	{
		PlayerRef->bIsHold = bState;
	}

	UFUNCTION(BlueprintCallable, Category="Command")
	FORCEINLINE void SetTargetHold( bool bState )
	{
		TargetRef->bIsHold = bState;
	}
	
private:
	class UDBSZEventManager* EventManager;
	bool bIsCameraResetting = false;

public:
	// ✅ 옆으로 피할 거리를 설정하는 변수를 추가합니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings|Obstruction")
	float ObstructionAvoidanceOffset = 150.f;

	// ✅ 일직선으로 판단할 민감도를 설정하는 변수를 추가합니다. (1.0에 가까울수록 민감)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings|Obstruction")
	float ObstructionDotThreshold = 0.98f;

	// ✅ 타겟 가림을 회피하는 위치를 계산할 새로운 함수를 선언합니다.
	FVector GetAvoidanceAdjustedCameraLocation();


	
protected:
	// ✅ 새로 추가: 리셋을 발동시킬 거리(cm/유닛)입니다. 에디터에서 조절하세요.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings|Alignment")
	float AlignmentResetThreshold = 200.f;

	// ✅ 새로 추가: 얼라인먼트를 체크할 새로운 함수를 선언합니다.
	bool ShouldResetByAlignment() const;

	// 참고: 기존의 DeadZonePlayer_X_Min 등의 변수들은 이제 이 로직에서 사용되지 않습니다.
};
