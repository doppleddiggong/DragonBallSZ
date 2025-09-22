// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.


#include "ASelectPawn.h"

#include "Components/CapsuleComponent.h"
#include "FCharacterAssetData.h"
#include "UCharacterData.h"
#include "UDBSZDataManager.h"
#include "Components/ArrowComponent.h"
#include "Materials/MaterialInstanceConstant.h"

ASelectPawn::ASelectPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	SetRootComponent(CapsuleComponent);
	
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(CapsuleComponent);

	ArrowComp = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComp"));
	ArrowComp->SetupAttachment(MeshComp);

	ArrowComp->SetRelativeLocation( FVector(0.0f, 0.0f, 80.f));
	ArrowComp->SetRelativeRotation( FRotator(0.0f,90,0.0f));
	ArrowComp->ArrowLength = 240;
}

void ASelectPawn::BeginPlay()
{
	Super::BeginPlay();
}

void ASelectPawn::SetupCharacterFromType(const ECharacterType Type, const bool Another)
{
	this->CharacterType = Type;
	this->bIsAnother = Another;
	
	FCharacterAssetData Params;
	UDBSZDataManager::Get(GetWorld())->GetCharacterAssetData(Type, Params);

	if (Params.CharacterDataAsset.IsValid())
		this->CharacterData = Params.CharacterDataAsset.Get();
	else
		this->CharacterData = Params.CharacterDataAsset.LoadSynchronous();

	if (CharacterData->MeshData.IsValid())
		MeshComp->SetSkeletalMesh(CharacterData->MeshData.Get());
	else
		MeshComp->SetSkeletalMesh(CharacterData->MeshData.LoadSynchronous());

	MeshComp->EmptyOverrideMaterials();
	for (int32 i = 0; i < CharacterData->MaterialArray.Num(); ++i)
	{
		UMaterialInstanceConstant* MaterialInst = nullptr;
		if (CharacterData->MaterialArray[i].IsValid())
			MaterialInst = CharacterData->MaterialArray[i].Get();
		else
			MaterialInst = CharacterData->MaterialArray[i].LoadSynchronous();

		if (MaterialInst)
		{
			auto DynamicMat = UMaterialInstanceDynamic::Create(MaterialInst, MeshComp);
			if (DynamicMat)
			{
				// 1번이 Cloth
				if (i == 1)
					DynamicMat->SetScalarParameterValue(FName("AnotherSide"), bIsAnother ? 1.0f : 0.0f);
				MeshComp->SetMaterial(i, DynamicMat);
			}
		}
	}
    
	MeshComp->SetRelativeLocation( CharacterData->RelativeLocation);
	MeshComp->SetRelativeRotation( CharacterData->RelativeRotator );
	MeshComp->SetRelativeScale3D( CharacterData->RelativeScale );
    
	if (CharacterData->AnimBluePrint.IsValid())
		MeshComp->SetAnimInstanceClass(CharacterData->AnimBluePrint.Get());
	else
		MeshComp->SetAnimInstanceClass(CharacterData->AnimBluePrint.LoadSynchronous());

	AnimInstance = MeshComp->GetAnimInstance();

	CharacterData->LoadIdleMontage(IdleMontage);
	CharacterData->LoadFocusMontage(FocusMontage);
	CharacterData->LoadSelectMontage(SelectMontage);

	PlayIdleAnimation();
}

void ASelectPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASelectPawn::SetSelectionState(ESelectionState NewState)
{
	CurrentSelectionState = NewState;
	if (CurrentSelectionState == ESelectionState::None)
	{
		PlayIdleAnimation();
	}
	else
	{
		PlaySelectAnimation();
	}
}

void ASelectPawn::PlayFocusAnimation()
{
	if (IsValid(AnimInstance) && IsValid(FocusMontage))
	{
		AnimInstance->Montage_Play(FocusMontage, 1.0f);
		FOnMontageEnded MontageEndedDelegate;
		MontageEndedDelegate.BindUObject(this, &ASelectPawn::OnFocusAnimationEnded);
		AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, FocusMontage);
	}
}

void ASelectPawn::PlayIdleAnimation()
{
	if (IsValid(AnimInstance) && IsValid(IdleMontage))
	{
		AnimInstance->Montage_Play(IdleMontage, 1.0f);
	}
}

void ASelectPawn::PlaySelectAnimation()
{
	if (IsValid(AnimInstance) && IsValid(SelectMontage))
	{
		AnimInstance->Montage_Play(SelectMontage, 1.0f);
	}
}

void ASelectPawn::OnFocusAnimationEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (bInterrupted)
	{
		return;
	}

	if (CurrentSelectionState == ESelectionState::None)
		PlayIdleAnimation();
	else
		PlaySelectAnimation();
}