// Fill out your copyright notice in the Description page of Project Settings.

#include "UICOinFlip.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "PAASchifanoFrancesco/Core/MyGameMode.h"
#include "Animation/WidgetAnimation.h"
#include "Kismet/GameplayStatics.h"

void UUICOinFlip::NativeConstruct()
{
	Super::NativeConstruct();

	GameMode = Cast<AMyGameMode>(UGameplayStatics::GetGameMode(this));
    
	if (GameMode && CoinFlipResultText)
	{
		FString ResultText = (GameMode->GetCoinFlipResult() == EPlayer::Player1) ? TEXT("Player Starts!") : TEXT("AI Starts!");
		CoinFlipResultText->SetText(FText::FromString(ResultText));
	}

	if (ButtonEasy)
		ButtonEasy->OnClicked.AddDynamic(this, &UUICOinFlip::OnEasyClicked);

	if (ButtonHard)
		ButtonHard->OnClicked.AddDynamic(this, &UUICOinFlip::OnHardClicked);

	if (CoinImage && FlipAnimation)
	{
		SetFlipAnimationSpeed(20.0f);
	}
}

void UUICOinFlip::OnEasyClicked()
{
	if (GameMode)
	{
		GameMode->AILevel = EAILevel::Easy;
		GameMode->SetGamePhase(EGamePhase::EPlacement);
	}
}

void UUICOinFlip::OnHardClicked()
{
	if (GameMode)
	{
		GameMode->AILevel = EAILevel::Hard;
		GameMode->SetGamePhase(EGamePhase::EPlacement);
	}
}

void UUICOinFlip::SetFlipAnimationSpeed(float Speed)
{
	if (FlipAnimation)
	{
		PlayAnimation(FlipAnimation, 0.0f, 0, EUMGSequencePlayMode::Forward, Speed);
	}
}