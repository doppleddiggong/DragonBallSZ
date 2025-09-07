// Copyright (c) 2025 Doppleddiggong. All rights reserved. Unauthorized copying, modification, or distribution of this file, via any medium is strictly prohibited. Proprietary and confidential.

#include "ACombatLevelScript.h"

#include "GameEvent.h"
#include "UCombatUI.h"
#include "UDBSZEventManager.h"
#include "Blueprint/UserWidget.h"
#include "Shared/FComponentHelper.h"

#define COMBAT_WIDGET_PATH TEXT("/Game/CustomContents/UI/WB_Combat.WB_Combat_C")

class UCombatUI;

ACombatLevelScript::ACombatLevelScript()
{
	CombatUIFactory = FComponentHelper::LoadClass<UUserWidget>(COMBAT_WIDGET_PATH);
}

void ACombatLevelScript::BeginPlay()
{
	Super::BeginPlay();
	
	CombatUI = CreateWidget<UCombatUI>(GetWorld(), CombatUIFactory);

	if (auto EventManager = UDBSZEventManager::Get(GetWorld()))
	{
		EventManager->OnMessage.AddDynamic(this, &ACombatLevelScript::OnRecvMessage);
		EventManager->SendMessage( GameEvent::GameStart.ToString() );
	}
}

void ACombatLevelScript::OnRecvMessage(FString InMsg)
{
	if ( InMsg.Equals(GameEvent::GameStart.ToString(), ESearchCase::IgnoreCase ))
	{
		this->GameStart();
	}
}

void ACombatLevelScript::GameStart()
{
	CombatTime = 0.0f;
	CombatUI->AddToViewport();
	
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(
		TimerHandle, 
		this, 
		&ACombatLevelScript::OnTimerTick, 
		1.0f,
		true,
		0.0f 
	);
}

void ACombatLevelScript::OnTimerTick()
{
	CombatTime += 1;

	if ( IsValid(CombatUI))
		CombatUI->UpdateTimer(CombatTime);
}