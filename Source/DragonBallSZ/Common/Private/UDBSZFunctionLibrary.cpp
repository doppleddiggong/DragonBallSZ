#include "UDBSZFunctionLibrary.h"

TSubclassOf<UDBSZDamageType> UDBSZFunctionLibrary::GetDamageTypeClass(EAttackPowerType InType)
{
    TSubclassOf<UDBSZDamageType> DamageTypeClass = UDBSZDamageType::StaticClass();
    if (DamageTypeClass)
    {
        UDBSZDamageType* DefaultDamageType = DamageTypeClass->GetDefaultObject<UDBSZDamageType>();
        if (DefaultDamageType)
            DefaultDamageType->AttackPowerType = InType;
    }
    return DamageTypeClass;
}
