// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UIMainMenu.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class PAASCHIFANOFRANCESCO_API UUIMainMenu : public UUserWidget
{
	GENERATED_BODY()

public:
    // Variabile per il TextBlock del titolo
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UTextBlock* TitleText;

    // Variabile per il Button di avvio
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UButton* StartGameButton;

    // Funzione che viene chiamata quando il widget viene costruito
    virtual void NativeConstruct() override;

    // Funzione da chiamare al click del pulsante
    UFUNCTION()
    void OnStartGameClicked();
};