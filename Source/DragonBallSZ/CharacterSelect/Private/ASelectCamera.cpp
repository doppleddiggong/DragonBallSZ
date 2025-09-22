// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "ASelectCamera.h"
#include "ASelectPawn.h"
#include "EAnimMontageType.h"
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

void ASelectCamera::BeginPlay()
{
	Super::BeginPlay();

	// "SelectTransform"을 이름에 포함하는 액터를 찾아 배열에 등록합니다.
	SelectTransforms.Empty();
	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

	for (AActor* Actor : AllActors)
	{
		if (Actor && Actor->GetActorLabel().Contains(TEXT("SelectTransform")))
		{
			SelectTransforms.Add(Actor);
		}
	}

	// 찾은 액터를 이름순으로 정렬하여 순서를 보장합니다.
	SelectTransforms.Sort([](const TObjectPtr<AActor>& A, const TObjectPtr<AActor>& B)
	{
		return A->GetActorLabel() < B->GetActorLabel();
	});

	// 기존 Pawn 배열을 비웁니다.
	SelectPawns.Empty();

	// 데이터와 트랜스폼 배열의 크기가 같은지 확인합니다.
	if (SelectData.Num() != SelectTransforms.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("ASelectCamera: SelectData count (%d) and found SelectTransforms count (%d) do not match!"), SelectData.Num(), SelectTransforms.Num());
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 i = 0; i < SelectData.Num(); ++i)
	{
		if (!SelectTransforms.IsValidIndex(i) || !SelectTransforms[i])
		{
			UE_LOG(LogTemp, Warning, TEXT("ASelectCamera: TransformActor at index %d is invalid."), i);
			continue;
		}
		
		const FSelectCharacterData& Data = SelectData[i];
		const AActor* TransformActor = SelectTransforms[i];

		const FTransform SpawnTransform = TransformActor->GetActorTransform();
		ASelectPawn* NewPawn = GetWorld()->SpawnActor<ASelectPawn>(ASelectPawn::StaticClass(), SpawnTransform, SpawnParams);

		if (NewPawn)
		{
			NewPawn->SetupCharacterFromType(Data.PowerType, Data.bIsAnother);
			SelectPawns.Add(NewPawn);
		}
	}

	if (SelectPawns.Num() > 0)
	{
		FocusCharacter(CurrentFocusIndex);
	}
}

void ASelectCamera::MoveCharacter(int32 Direction)
{
	if (SelectPawns.Num() == 0)
	{
		return;
	}

	CurrentFocusIndex += Direction;

	if (CurrentFocusIndex < 0)
	{
		CurrentFocusIndex = SelectPawns.Num() - 1;
	}
	else if (CurrentFocusIndex >= SelectPawns.Num())
	{
		CurrentFocusIndex = 0;
	}

	FocusCharacter(CurrentFocusIndex);
}

void ASelectCamera::FocusCharacter(int32 FocusIndex)
{
	if (!SelectPawns.IsValidIndex(FocusIndex))
	{
		TargetPawn = nullptr;
		return;
	}

	if ( TargetPawn != nullptr)
	{
		TargetPawn->PlayTypeMontage(EAnimMontageType::Idle);
	}

	TargetPawn = SelectPawns[FocusIndex];
	TargetPawn->PlayTypeMontage(EAnimMontageType::Focus);
}
