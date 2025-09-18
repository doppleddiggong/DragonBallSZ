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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(EditAnywhere)
	class USpringArmComponent* SpringArmComp;
	UPROPERTY(EditAnywhere)
	class UCameraComponent* CameraComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class APlayerActor* PlayerRef;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class AEnemyActor* TargetRef;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CameraDistance = 200;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TargetDistance = 500;
	float CurrentDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CameraResetTime = 5;
	
public:
	UFUNCTION(BlueprintCallable)
	void PlayerRotationLock();

	UFUNCTION(BlueprintCallable)
	void CloseCameraRotation(float DeltaTime);

	UFUNCTION(BlueprintCallable)
	void ResetCameraLocation(float DeltaTime);

	UFUNCTION(BlueprintCallable)
	void ResetCameraRotation(float DeltaTime);


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

private:
	class UDBSZEventManager* EventManager;
	bool bIsCameraResetting = false;

public:
	// ✅ 옆으로 피할 거리를 설정하는 변수를 추가합니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings|Obstruction")
	float ObstructionAvoidanceOffset = 200.f;

	// ✅ 일직선으로 판단할 민감도를 설정하는 변수를 추가합니다. (1.0에 가까울수록 민감)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings|Obstruction")
	float ObstructionDotThreshold = 0.9f;

	// ✅ 타겟 가림을 회피하는 위치를 계산할 새로운 함수를 선언합니다.
	FVector GetAvoidanceAdjustedCameraLocation();


	
protected:
	// ✅ 새로 추가: 리셋을 발동시킬 거리(cm/유닛)입니다. 에디터에서 조절하세요.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings|Alignment")
	float AlignmentResetThreshold = 200.f;

	// ✅ 새로 추가: 얼라인먼트를 체크할 새로운 함수를 선언합니다.
	bool ShouldResetByAlignment() const;


	/** 플레이어 컨트롤러에 대한 참조입니다. 화면 좌표 계산에 필요합니다. */
	UPROPERTY()
	TObjectPtr<APlayerController> PlayerControllerRef;
    
	/** 화면 가장자리로부터의 안전 영역(%)입니다. 0.1은 10%를 의미합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera | Screen Edge")
	float ScreenSafeZonePadding = 0.1f;

	/** 화면 가장자리에서 카메라를 밀어내는 속도입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera | Screen Edge")
	float ScreenEdgeCorrectionSpeed = 500.f;
};
