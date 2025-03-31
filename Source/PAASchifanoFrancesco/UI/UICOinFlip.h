// Creato da: Schifano Francesco, 5469994

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Include base per tutti i progetti Unreal
#include "CoreMinimal.h"

// Include della classe base per tutti i widget UMG
#include "Blueprint/UserWidget.h"

// Generazione automatica delle macro per riflessione e serializzazione
#include "UICoinFlip.generated.h"

// Forward declaration per evitare dipendenze pesanti nei file header
class UTextBlock;           // Blocco di testo per visualizzare il risultato del lancio
class UButton;              // Pulsanti per scegliere la difficoltà
class UImage;               // Immagine della moneta animata
class AMyGameMode;          // GameMode personalizzato del progetto

/**
 * Classe UUICOinFlip
 * Rappresenta il widget per il lancio della moneta e la scelta della difficoltà (Easy/Hard).
 */
UCLASS()
class PAASCHIFANOFRANCESCO_API UUICOinFlip : public UUserWidget
{
	GENERATED_BODY()

public:

	/**
	 * Override di NativeConstruct.
	 * Viene chiamato automaticamente quando il widget viene costruito in runtime.
	 * In questo metodo colleghiamo gli eventi dei pulsanti e impostiamo il testo del risultato.
	 */
	virtual void NativeConstruct() override;

	/**
	 * Metodo chiamato quando il pulsante "Easy" viene premuto.
	 * Imposta la difficoltà su "Easy" e passa alla fase di piazzamento.
	 */
	UFUNCTION()
	void OnEasyClicked();

	/**
	 * Metodo chiamato quando il pulsante "Hard" viene premuto.
	 * Imposta la difficoltà su "Hard" e passa alla fase di piazzamento.
	 */
	UFUNCTION()
	void OnHardClicked();

	/**
	 * Imposta la velocità dell’animazione della moneta.
	 * @param Speed Velocità con cui riprodurre l’animazione (più alto = più veloce).
	 */
	void SetFlipAnimationSpeed(float Speed);

protected:

	/** Blocco di testo che mostra chi inizia dopo il lancio della moneta */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CoinFlipResultText;

	/** Pulsante per selezionare la difficoltà "Easy" */
	UPROPERTY(meta = (BindWidget))
	UButton* ButtonEasy;

	/** Pulsante per selezionare la difficoltà "Hard" */
	UPROPERTY(meta = (BindWidget))
	UButton* ButtonHard;

	/** Immagine che rappresenta graficamente la moneta */
	UPROPERTY(meta = (BindWidget))
	UImage* CoinImage;

	/** Animazione della moneta (collegata tramite BindWidgetAnim) */
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* FlipAnimation;

private:

	/** Riferimento al GameMode attuale (recuperato tramite UGameplayStatics) */
	AMyGameMode* GameMode;
};