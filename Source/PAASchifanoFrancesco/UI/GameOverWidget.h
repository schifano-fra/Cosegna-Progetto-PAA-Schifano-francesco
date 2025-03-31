// Creato da: Schifano Francesco, 5469994

#pragma once

// Include base di Unreal Engine per tutti i progetti
#include "CoreMinimal.h"

// Include necessario per creare widget personalizzati
#include "Blueprint/UserWidget.h"

// Macro per generare il codice associato alla classe
#include "GameOverWidget.generated.h"

// Forward declaration delle classi UI utilizzate nel widget
class UTextBlock;
class UButton;

/**
 * UGameOverWidget
 * Widget visualizzato alla fine della partita.
 * Mostra il nome del vincitore e un pulsante per uscire dal gioco.
 */
UCLASS()
class PAASCHIFANOFRANCESCO_API UGameOverWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	/**
	 * Metodo: SetWinnerText
	 * Descrizione: Imposta il testo che mostra il vincitore della partita.
	 * @param Winner - Nome del vincitore (es. "Player" o "AI")
	 */
	UFUNCTION(BlueprintCallable)
	void SetWinnerText(const FString& Winner);

protected:

	/**
	 * Metodo: NativeConstruct
	 * Descrizione: Override del metodo nativo chiamato all'inizio della vita del widget.
	 * Utilizzato per il binding degli eventi (es. click sul pulsante).
	 */
	virtual void NativeConstruct() override;

	/**
	 * Metodo: OnClicked_Exit
	 * Descrizione: Funzione chiamata quando si clicca il pulsante di uscita.
	 * Chiude il gioco.
	 */
	UFUNCTION()
	void OnClicked_Exit();

protected:

	/** Riferimento al TextBlock che mostra il nome del vincitore. 
		Collegato al widget tramite BindWidget (deve avere lo stesso nome nel Blueprint). */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* WinnerText;

	/** Riferimento al pulsante di uscita. 
		Collegato al widget tramite BindWidget (deve avere lo stesso nome nel Blueprint). */
	UPROPERTY(meta = (BindWidget))
	UButton* ExitButton;
};
