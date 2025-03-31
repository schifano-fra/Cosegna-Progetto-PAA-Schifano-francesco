// Creato da: Schifano Francesco, 5469994

#pragma once

// Include base di Unreal Engine per i tipi fondamentali
#include "CoreMinimal.h"

// Include per poter creare e gestire widget utente personalizzati
#include "Blueprint/UserWidget.h"

// Macro necessaria per la generazione del codice da parte dell'Unreal Header Tool
#include "InfoWidget.generated.h"

// Forward declaration di classi usate nei puntatori UPROPERTY
class UButton;    // Pulsante per tornare al gioco
class UImage;     // Immagini che mostrano le informazioni

/**
 * Classe UInfoWidget
 * Rappresenta un widget informativo mostrato durante la fase di piazzamento o battaglia.
 * Contiene immagini statiche con le istruzioni e un pulsante per tornare alla partita.
 */
UCLASS()
class PAASCHIFANOFRANCESCO_API UInfoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Override del metodo NativeConstruct
	 * Viene chiamato automaticamente quando il widget è costruito e pronto all'uso.
	 * Serve per effettuare binding di eventi (es. click su pulsanti).
	 */
	virtual void NativeConstruct() override;

	/**
	 * Funzione che gestisce il click sul pulsante "Back to Game".
	 * Rimuove il widget dalla schermata e ripristina il controllo al gameplay.
	 */
	UFUNCTION()
	void OnBackClicked();

protected:
	/** 
	 * Riferimento al pulsante di ritorno al gioco. Deve essere assegnato via Blueprint.
	 * Questo pulsante chiama OnBackClicked() quando cliccato.
	 */
	UPROPERTY(meta = (BindWidget))
	UButton* BackButton;

	/**
	 * Immagine che mostra la prima sezione delle istruzioni.
	 * Può includere regole generali, comandi di gioco, ecc.
	 */
	UPROPERTY(meta = (BindWidget))
	UImage* FirstInformation;

	/**
	 * Immagine che mostra ulteriori istruzioni o dettagli aggiuntivi.
	 */
	UPROPERTY(meta = (BindWidget))
	UImage* SecondInformation;

	/**
	 * Immagine opzionale che mostra i dettagli delle pedine (Sniper, Brawler, ecc.).
	 */
	UPROPERTY(meta = (BindWidget))
	UImage* PawnDetails;
};