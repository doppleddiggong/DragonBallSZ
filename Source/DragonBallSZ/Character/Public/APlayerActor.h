// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "IControllable.h"
#include "APlayerActor.generated.h"

UCLASS()
class DRAGONBALLSZ_API APlayerActor : public ACharacter, public IControllable
{
	GENERATED_BODY()

public:
	APlayerActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public: // Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class UStatSystem* StatSystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class URushAttackSystem* RushAttackSystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class UArrowComponent* LeftHandComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class UArrowComponent* RightHandComp;

	
public: // Control Interface
	virtual void Cmd_Move(const FVector2D& Axis) override;
	virtual void Cmd_Look(const FVector2D& Axis) override;

	virtual void Cmd_Jump() override;
	virtual void Cmd_Dash() override;
	
	virtual void Cmd_LockOn() override;

	virtual void Cmd_ChargeKi(bool bPressed) override;
	virtual void Cmd_Guard(bool bPressed) override;
	virtual void Cmd_Vanish() override;
	
	virtual void Cmd_RushAttack() override;
	virtual void Cmd_EnergyBlast() override;
	
	virtual void Cmd_Kamehameha() override;


public: // Control Interface
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player")
	bool IsSprinting = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player")
	bool IsHit = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player")
	class AEnemyActor* TargetActor;
};
