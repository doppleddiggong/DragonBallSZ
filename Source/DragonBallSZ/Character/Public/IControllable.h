// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IControllable.generated.h"

UINTERFACE(MinimalAPI)
class UControllable : public UInterface
{
	GENERATED_BODY()
};

class IControllable
{
	GENERATED_BODY()

public:
	virtual void Cmd_Move(const FVector2D& Axis) = 0;		// 이동
	virtual void Cmd_Look(const FVector2D& Axis) = 0;		// 카메라 회전

	virtual void Cmd_Jump() = 0;							// 점프
	virtual void Cmd_Dash() = 0;							// 대시(부스트)
	
	virtual void Cmd_LockOn() = 0;							// 적 대상 락온 토글

	virtual void Cmd_ChargeKi(bool bPressed) = 0;			// 기 모으기
	virtual void Cmd_Guard(bool bPressed) = 0;				// 가드
	virtual void Cmd_Vanish() = 0;							// 회피
	
	virtual void Cmd_RushAttack() = 0;						// 근접 공격
	virtual void Cmd_EnergyBlast() = 0;						// 기탄 발사

	virtual void Cmd_Kamehameha() = 0;						// 카메하메 파
};
