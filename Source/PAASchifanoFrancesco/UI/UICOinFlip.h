// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UICoinFlip.generated.h"

class UTextBlock;
class UButton;
class UImage;
class AMyGameMode;

UCLASS()
class PAASCHIFANOFRANCESCO_API UUICOinFlip : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnEasyClicked();

	UFUNCTION()
	void OnHardClicked();

	void SetFlipAnimationSpeed(float Speed);

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CoinFlipResultText;

	UPROPERTY(meta = (BindWidget))
	UButton* ButtonEasy;

	UPROPERTY(meta = (BindWidget))
	UButton* ButtonHard;
	
	UPROPERTY(meta = (BindWidget))
	UImage* CoinImage;  // Immagine della moneta

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* FlipAnimation;

private:
	AMyGameMode* GameMode;
};