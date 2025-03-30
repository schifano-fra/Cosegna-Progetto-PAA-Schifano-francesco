// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PAASchifanoFrancesco/Core/MyGameMode.h"
#include "PAASchifanoFrancesco/Units/UnitBase.h"
#include "Blueprint/UserWidget.h"
#include "SelectPawn.generated.h"


class UButton;
class UTextBlock;

UCLASS()
class PAASCHIFANOFRANCESCO_API USelectPawn : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	
	void DisableButtonForPawn(TSubclassOf<AUnitBase> PawnType);
	bool AreAllButtonsDisabled() const;

protected:
	UPROPERTY(meta = (BindWidget))
	UButton* BrawlerButton;
	UPROPERTY(meta = (BindWidget))
	UButton* SniperButton;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* SniperText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* BrawlerText;

private:
	UFUNCTION()
	void OnBrawlerSelected();

	UFUNCTION()
	void OnSniperSelected();
	
	AMyGameMode* GM;
	APlacementManager* PlacementManager;
};
