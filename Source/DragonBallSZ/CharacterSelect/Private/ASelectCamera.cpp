// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "ASelectCamera.h"
#include "ASelectPawn.h"
#include "ADBSZGameMode.h"

#include "Camera/CameraComponent.h"
#include "Components/ArrowComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

ASelectCamera::ASelectCamera()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	SetRootComponent(CameraComponent);
}

void ASelectCamera::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
	for (AActor* Actor : AllActors)
	{
		if (Actor && Actor->ActorHasTag(FName(TEXT("SelectTransform"))))
			SelectTransforms.Add(Actor);
	}

	// SelectTransforms.Sort([](const TObjectPtr<AActor>& A, const TObjectPtr<AActor>& B) {
	// 	return A->GetActorLabel() < B->GetActorLabel();
	// });

	SelectPawns.Empty();
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 i = 0; i < SelectData.Num(); ++i)
	{
		if (SelectTransforms.IsValidIndex(i) && SelectTransforms[i])
		{
			const FTransform SpawnTransform = SelectTransforms[i]->GetActorTransform();
			ASelectPawn* NewPawn = GetWorld()->SpawnActor<ASelectPawn>(PawnFactory, SpawnTransform, SpawnParams);
			if (NewPawn)
			{
				NewPawn->SetupCharacterFromType(SelectData[i].PowerType, SelectData[i].bIsAnother);
				SelectPawns.Add(NewPawn);
			}
		}
	}
	
	PawnSelectionStates.Init(ESelectionState::None, SelectPawns.Num());
	if (SelectPawns.Num() > 0)
		FocusCharacter(CurrentFocusIndex);
}

void ASelectCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (TargetPawn.IsValid())
	{
		if (const auto Arrow = TargetPawn->ArrowComp)
		{
			const FVector TargetLocation = Arrow->GetComponentLocation() + Arrow->GetForwardVector() * Arrow->ArrowLength;
			const FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(TargetLocation, TargetPawn->GetActorLocation());

			const FVector NewLocation = FMath::VInterpTo(GetActorLocation(), TargetLocation, DeltaTime, CameraInterpSpeed);
			const FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, CameraInterpSpeed);

			SetActorLocationAndRotation(NewLocation, NewRotation);
		}
	}
}

void ASelectCamera::MoveCharacter(int32 Direction)
{
	if (SelectPawns.Num() == 0)
	{
		return;
	}

	const int32 OldFocusIndex = CurrentFocusIndex;

	CurrentFocusIndex += Direction;
	if (CurrentFocusIndex < 0)
		CurrentFocusIndex = SelectPawns.Num() - 1;
	else if (CurrentFocusIndex >= SelectPawns.Num())
		CurrentFocusIndex = 0;

	if (SelectPawns.IsValidIndex(OldFocusIndex) && SelectPawns[OldFocusIndex])
		SelectPawns[OldFocusIndex]->SetSelectionState(PawnSelectionStates[OldFocusIndex]);

	FocusCharacter(CurrentFocusIndex);
}

void ASelectCamera::FocusCharacter(int32 FocusIndex)
{
	if (!SelectPawns.IsValidIndex(FocusIndex))
	{
		TargetPawn = nullptr;
		return;
	}

	TargetPawn = SelectPawns[FocusIndex];
	if (TargetPawn.IsValid())
		TargetPawn->PlayFocusAnimation();
}

ESelectionState ASelectCamera::GetCurrentFocusSelectionState() const
{
	if (PawnSelectionStates.IsValidIndex(CurrentFocusIndex))
		return PawnSelectionStates[CurrentFocusIndex];
	return ESelectionState::None;
}

void ASelectCamera::SelectCurrentCharacter()
{
	if (!SelectPawns.IsValidIndex(CurrentFocusIndex))
		return;

	if (PawnSelectionStates[CurrentFocusIndex] != ESelectionState::None)
		return;

	if (CurrentTurn == ESelectionState::PlayerSelected && PlayerSelectedIndex == -1)
	{
		PlayerSelectedIndex = CurrentFocusIndex;
		PawnSelectionStates[CurrentFocusIndex] = ESelectionState::PlayerSelected;
		SelectPawns[CurrentFocusIndex]->SetSelectionState(ESelectionState::PlayerSelected);
		CurrentTurn = ESelectionState::EnemySelected;
	}
	else if (CurrentTurn == ESelectionState::EnemySelected && EnemySelectedIndex == -1)
	{
		if (CurrentFocusIndex == PlayerSelectedIndex)
		{
			return;
		}
		EnemySelectedIndex = CurrentFocusIndex;
		PawnSelectionStates[CurrentFocusIndex] = ESelectionState::EnemySelected;
		SelectPawns[CurrentFocusIndex]->SetSelectionState(ESelectionState::EnemySelected);
		CurrentTurn = ESelectionState::PlayerSelected;
	}
}

void ASelectCamera::DeselectCharacter()
{
	if (!SelectPawns.IsValidIndex(CurrentFocusIndex))
		return;

	const ESelectionState StateToDeselect = PawnSelectionStates[CurrentFocusIndex];

	if (StateToDeselect == ESelectionState::PlayerSelected)
	{
		PlayerSelectedIndex = -1;
		PawnSelectionStates[CurrentFocusIndex] = ESelectionState::None;
		SelectPawns[CurrentFocusIndex]->SetSelectionState(ESelectionState::None);
		CurrentTurn = ESelectionState::PlayerSelected;
	}
	else if (StateToDeselect == ESelectionState::EnemySelected)
	{
		EnemySelectedIndex = -1;
		PawnSelectionStates[CurrentFocusIndex] = ESelectionState::None;
		SelectPawns[CurrentFocusIndex]->SetSelectionState(ESelectionState::None);
		CurrentTurn = ESelectionState::EnemySelected;
	}
}

void ASelectCamera::PassSelectionDataToGameMode()
{
	if (!IsPlayerSelected() || !IsEnemySelected())
	{
		return;
	}

	ADBSZGameMode* GameMode = Cast<ADBSZGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GameMode)
	{
		const FSelectCharacterData& PlayerData = SelectData[PlayerSelectedIndex];
		const FSelectCharacterData& EnemyData = SelectData[EnemySelectedIndex];
		GameMode->StartGameWithCharacters(PlayerData.PowerType, PlayerData.bIsAnother, EnemyData.PowerType, EnemyData.bIsAnother);
	}
}