// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "EBodyPartType.h"
#include "GameFramework/Character.h"
#include "AEnemyActor.generated.h"

UCLASS()
class DRAGONBALLSZ_API AEnemyActor : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyActor();

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="Enemy")
	void OnSightDetect(bool Target);

public:
	virtual void Tick(float DeltaTime) override;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class UArrowComponent* LeftHandComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class UArrowComponent* RightHandComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class UArrowComponent* LeftFootComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class UArrowComponent* RightFootComp;

public:
	FORCEINLINE UArrowComponent* GetBodyPart(EBodyPartType Part) const
	{
		switch (Part)
		{
		case EBodyPartType::Hand_L: return LeftHandComp;
		case EBodyPartType::Hand_R: return RightHandComp;
		case EBodyPartType::Foot_L: return LeftFootComp;
		case EBodyPartType::Foot_R: return RightFootComp;
		default:	return LeftHandComp;
		}
	}

	UFUNCTION(BlueprintCallable, Category="Enemy|Attack")
	void GetBodyLocation(USceneComponent* SceneComp, FVector& OutStart, FVector& OutEnd) const;
	
public: // Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy")
	class UStatSystem* StatSystem;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class UHitStopSystem* HitStopSystem;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy")
	class USightSystem* SightSystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy")
	class UEnemyFSM* EnemyFSM;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy")
	class UEnemyAnimInstance* AnimBP;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy")
	class AEnemyAIController* AIEnemy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy")
	class APlayerActor* TargetActor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy")
	bool IsHit = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy")
	float RotateLerpSpeed = 5.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy|Attack")
	float TraceLength  = 30.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy|Attack")
	float TraceRadius  = 30.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy|Attack")
	float TraceDrawTime = 1.5f;
};



