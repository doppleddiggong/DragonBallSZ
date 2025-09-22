// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "ECharacterType.h"
#include "ESelectionState.h"
#include "GameFramework/Pawn.h"
#include "ASelectPawn.generated.h"

UCLASS()
class DRAGONBALLSZ_API ASelectPawn : public APawn
{
	GENERATED_BODY()

public:
	ASelectPawn();

	
protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

public:
	UFUNCTION(BlueprintCallable, Category="SelectPawn")
	void SetupCharacterFromType(const ECharacterType Type, const bool Another);

	void SetSelectionState(ESelectionState NewState);

	void PlayFocusAnimation();
	void PlayIdleAnimation();
	void PlaySelectAnimation();

	UFUNCTION()
	void OnFocusAnimationEnded(UAnimMontage* Montage, bool bInterrupted);
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCapsuleComponent> CapsuleComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SelectPawn")
	TObjectPtr<class USkeletalMeshComponent> MeshComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="SelectPawn")
	TObjectPtr<class UAnimInstance> AnimInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SelectPawn")
	TObjectPtr<class UArrowComponent> ArrowComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character")
	TObjectPtr<class UCharacterData> CharacterData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	ECharacterType CharacterType = ECharacterType::Songoku;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	bool bIsAnother = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	ESelectionState CurrentSelectionState = ESelectionState::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|Montage")
	TObjectPtr<class UAnimMontage> IdleMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|Montage")
	TObjectPtr<class UAnimMontage> FocusMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|Montage")
	TObjectPtr<class UAnimMontage> SelectMontage;
};
