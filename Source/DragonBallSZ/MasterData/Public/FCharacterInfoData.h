#pragma once

#include "CoreMinimal.h"
#include "ECharacterType.h"
#include "Engine/DataTable.h"
#include "FCharacterInfoData.generated.h"

USTRUCT(BlueprintType)
struct FCharacterInfoData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ToolTip="캐릭터의 고유 식별 타입입니다. 데이터 테이블의 행 키로 사용됩니다."))
	ECharacterType Type = ECharacterType::Songoku;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|HP", meta=(ToolTip="캐릭터의 현재 체력입니다."))
	float CurHP = 1000;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|HP", meta=(ToolTip="캐릭터의 최대 체력입니다."))
	float MaxHP = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Attack", meta=(ToolTip="콤보 공격의 각 단계별 대미지 값입니다. 배열의 인덱스가 콤보 순서에 해당합니다."))
	TArray<float> AttackDamage { 30, 45, 50, 70, 100 };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Attack", meta=(ToolTip="콤보 공격 시 각 단계별로 회복되는 기(Ki)의 양입니다."))
	TArray<float> AttackChargeKi  { 3, 4, 5, 7, 10, 10, 10, 10, 10 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Ki", meta=(ToolTip="캐릭터가 전투 시작 시 보유하는 기(Ki)의 양입니다."))
	float StartKi = 300;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Ki", meta=(ToolTip="캐릭터가 보유할 수 있는 최대 기(Ki)의 양입니다."))
	float MaxKi = 500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Blast", meta=(ToolTip="기본 기탄(Blast)을 사용하는 데 필요한 기(Ki)의 양입니다."))
	float BlastNeedKi = 20;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Blast", meta=(ToolTip="기본 기탄(Blast)의 대미지입니다."))
	float BlastDamage = 30;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Blast", meta=(ToolTip="기본 기탄(Blast)의 연사 간격(딜레이)입니다."))
	float BlastShotDelay = 0.5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Kamehame", meta=(ToolTip="에네르기파(Kamehameha)를 사용하는 데 필요한 기(Ki)의 양입니다."))
	float KamehameNeedKi = 100;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Kamehame", meta=(ToolTip="에네르기파(Kamehameha)의 총 대미지입니다."))
	float KamehameDamage = 300;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Sight", meta=(ToolTip="AI가 적을 인지할 수 있는 최대 거리입니다."))
	float SightLength = 1000;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Sight", meta=(ToolTip="AI가 적을 인지할 수 있는 시야각(도)입니다."))
	float SightAngle = 45;	
};
