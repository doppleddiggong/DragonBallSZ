// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#pragma once

#include "CoreMinimal.h"
#include "ECharacterType.h"
#include "ESelectionState.h"
#include "GameFramework/Pawn.h"
#include "ASelectCamera.generated.h"


USTRUCT(BlueprintType)
struct FSelectCharacterData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ECharacterType PowerType = ECharacterType::Songoku;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsAnother = false;
};

UCLASS()
class DRAGONBALLSZ_API ASelectCamera : public APawn
{
	GENERATED_BODY()

public:
	ASelectCamera();

	// Called from UI
	UFUNCTION(BlueprintCallable, Category = "SelectCamera")
	void MoveCharacter(int32 Direction);
	
	UFUNCTION(BlueprintCallable, Category = "SelectCamera")
	void SelectCurrentCharacter();

	UFUNCTION(BlueprintCallable, Category = "SelectCamera")
	void DeselectCharacter();

	UFUNCTION(BlueprintPure, Category = "SelectCamera")
	bool IsPlayerSelected() const { return PlayerSelectedIndex != -1; }

	UFUNCTION(BlueprintPure, Category = "SelectCamera")
	bool IsEnemySelected() const { return EnemySelectedIndex != -1; }

	UFUNCTION(BlueprintPure, Category = "SelectCamera")
	ESelectionState GetCurrentFocusSelectionState() const;

	UFUNCTION(BlueprintCallable, Category = "SelectCamera")
	void PassSelectionDataToGameMode();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	void FocusCharacter(int32 FocusIndex);

public:
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Select")
	// TArray<TObjectPtr<AActor>> SelectTransforms;
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Data", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> CameraComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Select", meta = (AllowPrivateAccess = "true"))
	TArray<FSelectCharacterData> SelectData
	{
		FSelectCharacterData(ECharacterType::Songoku, false),
		FSelectCharacterData(ECharacterType::Songoku, true),

		FSelectCharacterData(ECharacterType::Vegeta, false),
		FSelectCharacterData(ECharacterType::Vegeta, true )
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Select", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class ASelectPawn> PawnFactory;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Select", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<class ASelectPawn>> SelectPawns;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SelectCamera", meta = (AllowPrivateAccess = "true"))
	int32 CurrentFocusIndex = 0;

	UPROPERTY()
	TWeakObjectPtr<class ASelectPawn> TargetPawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SelectCamera", meta = (AllowPrivateAccess = "true"))
	float CameraInterpSpeed = 5.0f;

	// Selection State
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SelectCamera", meta = (AllowPrivateAccess = "true"))
	int32 PlayerSelectedIndex = -1;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SelectCamera", meta = (AllowPrivateAccess = "true"))
	int32 EnemySelectedIndex = -1;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SelectCamera", meta = (AllowPrivateAccess = "true"))
	ESelectionState CurrentTurn = ESelectionState::PlayerSelected;

	UPROPERTY()
	TArray<ESelectionState> PawnSelectionStates;
};

