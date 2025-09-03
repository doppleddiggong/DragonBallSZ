// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "APlayerActor.generated.h"

UCLASS()
class DRAGONBALLSZ_API APlayerActor : public ACharacter
{
	GENERATED_BODY()

public:
	APlayerActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
