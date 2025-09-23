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
	ADynamicCameraActor();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
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
	void OnAttack(AActor* Target, int ComboCount);

private:
	UPROPERTY()
	TObjectPtr<UDBSZEventManager> EventManager;
	
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

	/* 캐릭터가 화면의 안전 영역 내에 있는지 확인하는 함수 */
	bool IsPlayerInSafeFrame(FVector2D& OutMin, FVector2D& OutMax) const;

	/** 높이에 따른 Z-Offset 조절을 위한 설정 값들입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings|Vertical Offset")
	float BaseSpringArmZOffset = 50.0f;

	/** 이 높이 차이(절대값)를 넘어서면 Z-Offset이 최대/최소치에 도달합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings|Vertical Offset")
	float MaxHeightDifference = 1000.0f;

	/** Base Z-Offset에서 위/아래로 움직일 수 있는 최대 범위입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings|Vertical Offset")
	float ZOffsetRange = 100.0f;

	/** Z-Offset 값이 변경될 때의 보간 속도입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings|Vertical Offset")
	float ZOffsetInterpSpeed = 3.0f;

private:
	/** 매 프레임 스프링암의 Z 오프셋을 업데이트하는 함수입니다. */
	void UpdateSpringArmOffset(float DeltaTime);
};
