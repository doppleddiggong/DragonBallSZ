// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "EAttackPowerType.h"
#include "ESoundType.h"
#include "Engine/DataAsset.h"
#include "UCharacterData.generated.h"

USTRUCT(BlueprintType)
struct FRushData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EAttackPowerType PowerType = EAttackPowerType::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UAnimMontage> AttackAsset;
};

UCLASS(BlueprintType)
class DRAGONBALLSZ_API UCharacterData : public UDataAsset
{
	GENERATED_BODY()

public:
	bool LoadRushAttackMontage(TArray<TObjectPtr<UAnimMontage>>& OutMontage, TArray<EAttackPowerType>& OutPowerType ) const;
	bool LoadDashMontage( TObjectPtr<UAnimMontage>& OutDashMontage) const;
	bool LoadHitMontage( TArray<TObjectPtr<UAnimMontage>>& OutHitMontage) const;
	bool LoadDeathMontage( TObjectPtr<UAnimMontage>& OutDeathMontage) const;
	bool LoadBlastMontage(TArray<TObjectPtr<UAnimMontage>>& OutMontage ) const;
	bool LoadChargeKiMontage( TObjectPtr<UAnimMontage>& OutMontage) const;
	bool LoadKamehameMontage( TObjectPtr<UAnimMontage>& OutMontage) const;
	bool LoadIntroMontage( TObjectPtr<UAnimMontage>& OutMontage) const;
	bool LoadWinMontage( TObjectPtr<UAnimMontage>& OutMontage) const;
	bool LoadIdleMontage( TObjectPtr<UAnimMontage>& OutMontage) const;
	bool LoadFocusMontage( TObjectPtr<UAnimMontage>& OutMontage) const;
	bool LoadSelectMontage( TObjectPtr<UAnimMontage>& OutMontage) const;
	

	
	bool LoadDashVFX( TObjectPtr<class UNiagaraSystem>& OutVFX) const;
	bool LoadChargeKiVFX( TObjectPtr<class UNiagaraSystem>& OutVFX) const;

	bool LoadEnergyBlast(TSubclassOf<class AEnergyBlastActor>& OutEnergyBlast);
	bool LoadKamehame(TSubclassOf<class AKamehamehaActor>& OutActor);

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Skeletal)
	TSoftObjectPtr<class USkeletalMesh> MeshData;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Skeletal)
	FVector RelativeLocation = FVector(0,0,-88.0f);
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Skeletal)
	FRotator RelativeRotator = FRotator(0,0,-90.0f);
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Skeletal)
	FVector RelativeScale = FVector(1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Skeletal)
	TArray<TSoftObjectPtr<UMaterialInstanceConstant>> MaterialArray;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Socket)
	FName LeftHandSocketName = FName("hand_l");
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Socket)
	FName RightHandSocketName = FName("hand_r");
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Socket)
	FName LeftFootSocketName = FName("foot_l");
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Socket)
	FName RightFootSocketName = FName("foot_r");
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=AnimBlueprint)
	TSoftClassPtr<class UAnimInstance> AnimBluePrint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Sound)
	ESoundType SoundAttack;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Sound)
	ESoundType SoundHit;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Sound)
	ESoundType SoundJump;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Sound)
	ESoundType SoundTeleport;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Sound)
	ESoundType SoundWin;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Montage)
	TArray<FRushData> RushData;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Montage)
	TSoftObjectPtr<class UAnimMontage> DashAsset;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Montage)
	TArray<TSoftObjectPtr<class UAnimMontage>> HitAsset;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Montage)
	TSoftObjectPtr<class UAnimMontage> DeathAsset;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Montage)
	TArray<TSoftObjectPtr<class UAnimMontage>> BlastAsset;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Montage)
	TSoftObjectPtr<class UAnimMontage> ChargeKiAsset;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Montage)
	TSoftObjectPtr<class UAnimMontage> KamehameAsset;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Montage)
	TSoftObjectPtr<class UAnimMontage> IntroAsset;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Montage)
	TSoftObjectPtr<class UAnimMontage> WinAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Montage)
	TSoftObjectPtr<class UAnimMontage> IdleAsset;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Montage)
	TSoftObjectPtr<class UAnimMontage> FocusAsset;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Montage)
	TSoftObjectPtr<class UAnimMontage> SelectAsset;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=VFX)
	TSoftObjectPtr<class UNiagaraSystem> DashVFX;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=VFX)
	TSoftObjectPtr<class UNiagaraSystem> ChargeKiVFX;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Actor)
	TSubclassOf<class AEnergyBlastActor> EnergyBlast;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Actor)
	TSubclassOf<class AKamehamehaActor> Kamehame;
};