#pragma once

#include "CoreMinimal.h"
#include "EAttackPowerType.h"
#include "Engine/DataTable.h"
#include "FKnockbackData.generated.h"

USTRUCT(BlueprintType)
struct FKnockbackData : public FTableRowBase
{
	GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ToolTip="공격 강도 타입(행 키)"))
    EAttackPowerType Type = EAttackPowerType::Normal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", ToolTip="통합 넉백 파워(cm/s). 공격자→피격자 방향에 곱해 사용"))
    float KnockbackPower = 1500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ToolTip="수직 발사 속도(cm/s). +위로 / -아래로"))
    float UpPower = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", ToolTip="넉백 유지 시간(초). 경과 후 마찰 계수 복원"))
    float Duration = 0.22f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", ClampMax="1", ToolTip="넉백 중 브레이킹 마찰 계수(0~1)"))
    float BrakingFrictionFactor = 0.30f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ToolTip="지정된 시간 후 플라잉 모드로 전환"))
    bool bAfterFlying = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", EditCondition="bAfterFlying", ToolTip="플라잉 전환 지연 시간(초)"))
    float FlyingDelay = 0.0f;
};
