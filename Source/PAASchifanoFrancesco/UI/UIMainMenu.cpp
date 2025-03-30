// Fill out your copyright notice in the Description page of Project Settings.

#include "UIMainMenu.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "PAASchifanoFrancesco/Core/MyGameMode.h"
#include "Kismet/GameplayStatics.h"

void UUIMainMenu::NativeConstruct()
{
	Super::NativeConstruct();

	if (StartGameButton)
	{
		StartGameButton->OnClicked.AddDynamic(this, &UUIMainMenu::OnStartGameClicked);
		
		if (UTextBlock* ButtonText = Cast<UTextBlock>(StartGameButton->GetChildAt(0)))
		{
			ButtonText->SetText(FText::FromString("Start game!"));
		}
	}
}

void UUIMainMenu::OnStartGameClicked()
{
	AMyGameMode* GameMode = Cast<AMyGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		RemoveFromParent();
		GameMode->SetGamePhase(EGamePhase::ECoinFlip);
	}
}