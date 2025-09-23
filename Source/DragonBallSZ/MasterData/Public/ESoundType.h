#pragma once

UENUM(BlueprintType)
enum class ESoundType : uint8
{
	Goku_Attack				UMETA(DisplayName = "오공 공격 Small"),
	Goku_Hit				UMETA(DisplayName = "오공 피격"),
	Goku_Jump				UMETA(DisplayName = "오공 점프"),
	Goku_Teleport			UMETA(DisplayName = "오공 텔레포트"),
	Goku_Win				UMETA(DisplayName = "오공 승리"),
		
	Vege_Attack				UMETA(DisplayName = "베지터 공격 Small"),
	Vege_Hit				UMETA(DisplayName = "베지터 피격"),
	Vege_Jump				UMETA(DisplayName = "베지터 점프"),
	Vege_Teleport			UMETA(DisplayName = "베지터 텔레포트"),
	Vege_Win				UMETA(DisplayName = "베지터 승리"),

	EnergyBlast_Fire		UMETA(DisplayName = "조기탄 발사"),
	EnergyBlast_Explosion	UMETA(DisplayName = "조기탄 폭발"),

	Kamehameha_Charge		UMETA(DisplayName = "에네르기파 차지"),
	Kamehameha_Fire			UMETA(DisplayName = "에네르기파 발사"),
	Kamehameha_Continue		UMETA(DisplayName = "에네르기파 지속"),
	Kamehameha_Explosion	UMETA(DisplayName = "에네르기파 폭발"),

	Goku_Kamehameha			UMETA(DisplayName = "오공 카메하메"),
	Goku_Kamehameha_Pa		UMETA(DisplayName = "오공 카메하메 파"),

	Vege_Kamehameha			UMETA(DisplayName = "베지터 카메하메"),
	Vege_Kamehameha_Pa		UMETA(DisplayName = "베지터 카메하메 파"),

	Goku_Lose				UMETA(DisplayName = "오공 패배"),
	Vege_Lose				UMETA(DisplayName = "베지터 패배"),

	Goku_Vs_Vege 			UMETA(DisplayName = "오공 VS 베지터"),
	Vege_Vs_Goku			UMETA(DisplayName = "베지터 VS 오공"),

	Goku_Attack02			UMETA(DisplayName = "오공 공격 Normal"),
	Goku_Attack03			UMETA(DisplayName = "오공 공격 Large"),
	Goku_Attack04			UMETA(DisplayName = "오공 공격 Huge"),
	
	Vege_Attack02			UMETA(DisplayName = "베지터 공격 Normal"),
	Vege_Attack03			UMETA(DisplayName = "베지터 공격 Large"),
	Vege_Attack04			UMETA(DisplayName = "베지터 공격 Huge"),

	Goku_ChargeKi			UMETA(DisplayName = "오공 기차지 음성"),
	Goku_VFX_ChargeKi		UMETA(DisplayName = "오공 기차지 효과음"),

	Vege_ChargeKi			UMETA(DisplayName = "베지터 기차지 음성"),
	Vege_VFX_ChargeKi		UMETA(DisplayName = "베지터 기차지 효과음"),
};
