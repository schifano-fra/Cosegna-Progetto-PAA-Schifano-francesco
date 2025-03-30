// Fill out your copyright notice in the Description page of Project Settings.

#include "SelectPawn.h"
#include "PAASchifanoFrancesco/Core/MyGameMode.h"
#include "PAASchifanoFrancesco/Core/PlacementManager.h"
#include "PAASchifanoFrancesco/Units/Sniper.h"
#include "PAASchifanoFrancesco/Units/Brawler.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void USelectPawn::NativeConstruct()
{
	Super::NativeConstruct();

	GM = Cast<AMyGameMode>(UGameplayStatics::GetGameMode(this));
	if (GM)
	{
		PlacementManager = GM->GetPlacementManager();
	}
	
	if (BrawlerButton) BrawlerButton->OnClicked.AddDynamic(this, &USelectPawn::OnBrawlerSelected);
	if (SniperButton) SniperButton->OnClicked.AddDynamic(this, &USelectPawn::OnSniperSelected);
}

FReply USelectPawn::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (BrawlerButton && BrawlerButton->IsHovered())
		{
			OnBrawlerSelected();
			return FReply::Handled();
		}
		else if (SniperButton && SniperButton->IsHovered())
		{
			OnSniperSelected();
			return FReply::Handled();
		}
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void USelectPawn::OnBrawlerSelected()
{
	PlacementManager->SetSelectedPawnType(ABrawler::StaticClass(), EPlayer::Player1);
}

void USelectPawn::OnSniperSelected()
{
	PlacementManager->SetSelectedPawnType(ASniper::StaticClass(), EPlayer::Player1);
}

void USelectPawn::DisableButtonForPawn(TSubclassOf<AUnitBase> PawnType)
{
	if (PawnType == ABrawler::StaticClass() && BrawlerButton)
	{
		BrawlerButton->SetIsEnabled(false);
	}
	else if (PawnType == ASniper::StaticClass() && SniperButton)
	{
		SniperButton->SetIsEnabled(false);
	}
}

bool USelectPawn::AreAllButtonsDisabled() const
{
	return (!BrawlerButton || !BrawlerButton->GetIsEnabled()) && (!SniperButton || !SniperButton->GetIsEnabled());
}
