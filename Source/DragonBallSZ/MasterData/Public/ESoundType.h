#pragma once

UENUM(BlueprintType)
enum class ESoundType : uint8
{
	Goku_Attack		UMETA(DisplayName = "오공 공격"),
	Goku_Hit		UMETA(DisplayName = "오공 피격"),
	Goku_Jump		UMETA(DisplayName = "오공 점프"),
	Goku_Teleport	UMETA(DisplayName = "오공 텔레포트"),
	Goku_Win		UMETA(DisplayName = "오공 승리"),

	Vege_Attack		UMETA(DisplayName = "베지터 공격"),
	Vege_Hit		UMETA(DisplayName = "베지터 피격"),
	Vege_Jump		UMETA(DisplayName = "베지터 점프"),
	Vege_Teleport	UMETA(DisplayName = "베지터 텔레포트"),
	Vege_Win		UMETA(DisplayName = "베지터 승리"),

	EnergyBlast_Fire UMETA(DisplayName = "조기탄 발사"),
	EnergyBlast_Explosion UMETA(DisplayName = "조기탄 폭발"),

	Kamehameha_Charge UMETA(DisplayName = "에네르기파 차지"),
	Kamehameha_Fire UMETA(DisplayName = "에네르기파 발사"),
	Kamehameha_Continue UMETA(DisplayName = "에네르기파 지속"),
	Kamehameha_Explosion UMETA(DisplayName = "에네르기파 폭발"),
};
