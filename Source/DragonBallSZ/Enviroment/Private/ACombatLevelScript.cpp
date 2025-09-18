// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "ACombatLevelScript.h"

#include <UDBSZSoundManager.h>

#include "ADynamicCameraActor.h"
#include "AEnemyActor.h"
#include "APlayerActor.h"
#include "UCombatUI.h"

#include "GameEvent.h"
#include "UDBSZEventManager.h"
#include "DragonBallSZ.h"
#include "EngineUtils.h"
#include "ESoundType.h"

#include "LevelSequencePlayer.h"
#include "MovieSceneSequencePlaybackSettings.h"

#include "Blueprint/UserWidget.h"
#include "Features/UDelayTaskManager.h"
#include "Shared/FComponentHelper.h"
#include "Kismet/GameplayStatics.h"

#define COMBAT_WIDGET_PATH TEXT("/Game/CustomContents/UI/WB_Combat.WB_Combat_C")

// #define MAIN_SEQ_PATH	TEXT("/Game/DynamicCamera/BattleIntro/MainSequence.MainSequence")
// #define GOKU_SEQ_PATH	TEXT("/Game/DynamicCamera/BattleOutro/GokuWin.GokuWin")

class ULevelSequencePlayer;

ACombatLevelScript::ACombatLevelScript()
{
	CombatUIFactory = FComponentHelper::LoadClass<UUserWidget>(COMBAT_WIDGET_PATH);
}

void ACombatLevelScript::BeginPlay()
{
	Super::BeginPlay();

	// MainSeq = LoadObject<ULevelSequence>(nullptr, MAIN_SEQ_PATH);
	// GokuWinSeq = LoadObject<ULevelSequence>(nullptr, GOKU_SEQ_PATH);
	
	if ( AActor* Camera = UGameplayStatics::GetActorOfClass(GetWorld(), ADynamicCameraActor::StaticClass()) )
		DynamicCameraActor = Cast<ADynamicCameraActor>(Camera);
	if ( AActor* Player = UGameplayStatics::GetActorOfClass( GetWorld(), APlayerActor::StaticClass() ) )
		PlayerActor = Cast<APlayerActor>(Player);
	if ( AActor* Enemy = UGameplayStatics::GetActorOfClass( GetWorld(), AEnemyActor::StaticClass() ) )
		EnemyActor = Cast<AEnemyActor>(Enemy);

	for( TActorIterator<AActor> it(GetWorld()); it; ++it )
	{
		auto Spawn = *it;
		if ( Spawn->GetActorNameOrLabel().Contains((TEXT("BP_GokuWin"))))
			GokuWinActor = Spawn;
	}
	
	EventManager = UDBSZEventManager::Get(GetWorld());
	EventManager->OnMessage.AddDynamic(this, &ACombatLevelScript::OnRecvMessage);
}

void ACombatLevelScript::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (EventManager)
	{
		EventManager->OnMessage.RemoveDynamic(this, &ACombatLevelScript::OnRecvMessage);
	}
}

void ACombatLevelScript::OnRecvMessage(FString InMsg)
{
	if ( InMsg.Equals(GameEvent::GameStart.ToString(), ESearchCase::IgnoreCase ))
	{
		PlayerActor->SetHold(true);
		EnemyActor->SetHold(true);

		ShowCombatUI(ESlateVisibility::Hidden);
	}
	else if ( InMsg.Equals(GameEvent::CombatStart.ToString(), ESearchCase::IgnoreCase ))
	{
		bCombatStart = true;
		bCombatResult = false;

		PlayerActor->SetHold(false);
		EnemyActor->SetHold(false);

		ShowCombatUI(ESlateVisibility::Visible);
	}
	else if ( InMsg.Equals(GameEvent::PlayerWin.ToString(), ESearchCase::IgnoreCase ))
	{
		GokuWinActor->SetActorTransform( PlayerActor->GetActorTransform());
		CombatResultProcess(true);
	}
	else if ( InMsg.Equals(GameEvent::EnemyWin.ToString(), ESearchCase::IgnoreCase ))
	{
		CombatResultProcess(false);
	}
}


void ACombatLevelScript::CombatResultProcess(bool IsPlayerWin)
{
	if ( bCombatResult )
		return;

	bCombatResult = true;
}

void ACombatLevelScript::ShowCombatUI(const ESlateVisibility InSetVisiblity)
{
	if ( CombatUI == nullptr )
	{
		CombatUI = CreateWidget<UCombatUI>(GetWorld(), CombatUIFactory);
		CombatUI->AddToViewport();
	}

	if ( !IsValid( CombatUI ) )
	{
		PRINTLOG(TEXT("CombatUI is nullptr, Please Check This State"));
		return;
	}

	CombatUI->SetVisibility(InSetVisiblity);

	if( InSetVisiblity == ESlateVisibility::Visible )
	{
		CombatUI->StartCombat(
			PlayerActor->StatSystem->CurHP/PlayerActor->StatSystem->MaxHP,
			EnemyActor->StatSystem->CurHP/EnemyActor->StatSystem->MaxHP );
	}
	else if( InSetVisiblity == ESlateVisibility::Hidden )
		CombatUI->ClearCombatTime();
}


void ACombatLevelScript::PlaySequence(class ULevelSequence* InSequence)
{
	// 이미 플레이어가 존재하면 먼저 정리
	if (SequencePlayer)
	{
		SequencePlayer->Stop();
		SequencePlayer = nullptr; // 가비지 컬렉터 대상이 되도록 nullptr로 설정
	}
	
	if (InSequence)
	{
		PlayingSequence = InSequence;
		SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(
			GetWorld(),
			InSequence,
			FMovieSceneSequencePlaybackSettings(),
			SequenceActor );
        
		if (SequencePlayer)
		{
			// **OnFinished 델리게이트에 함수 바인딩**
			SequencePlayer->OnFinished.AddDynamic(this, &ACombatLevelScript::OnSequenceFinished);
			
			SequencePlayer->Play();
			if (InSequence == GokuWinSeq)
			{
				UDBSZSoundManager::Get(GetWorld())->PlaySound2D(ESoundType::Goku_Win);
			}
			else if ( (InSequence == VegeWinSeq))
			{
				UDBSZSoundManager::Get(GetWorld())->PlaySound2D(ESoundType::Vege_Win);
			}
		}
	}
}

void ACombatLevelScript::OnSequenceFinished()
{
	// 어떤 시퀀스가 완료되었는지 확인
	if (PlayingSequence == MainSeq)
	{
		PRINTLOG(TEXT("MainSeq finished!"));

		if( auto TaskManager = UDelayTaskManager::Get(GetWorld()) )
		{
			UDelayTaskManager::Get(this)->Delay(this, CombatStartDelay, [this](){
				EventManager->SendMessage(GameEvent::CombatStart.ToString());
			});
		}

        if( auto PlayerController = UGameplayStatics::GetPlayerController(this, 0) )
            PlayerController->SetViewTargetWithBlend(DynamicCameraActor, 0.0f);
	}
	else if (PlayingSequence == GokuWinSeq)
	{
		PRINTLOG(TEXT("GokuWinSeq finished!"));
	}
	else
	{
		PRINTLOG(TEXT("Unknown Seq finished!"));
	}

	PlayingSequence = nullptr;

	if (SequencePlayer)
		SequencePlayer = nullptr;
}
