// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EBodyPartType.h"
#include "ICombatActor.generated.h"

UINTERFACE(MinimalAPI)
class UCombatActor : public UInterface
{
    GENERATED_BODY()
};

class ICombatActor
{
    GENERATED_BODY()

public:
    // Self as ACharacter
    class ACharacter* GetAsCharacter() const;

    // Target get/set
    virtual class ACharacter* GetTargetActor() const;
    virtual void SetTargetActor(class ACharacter* InTarget);

    // Capability & state
    virtual bool IsAttackEnable() const;
    virtual bool IsInSight(const AActor* Other) const;
    virtual bool GetIsHit() const;
    virtual void SetIsHit(bool bHit);
    virtual bool ShouldLookAtTarget() const;

    // Body part helper
    virtual class USceneComponent* GetBodyPart(EBodyPartType Part) const;
};