// Creato da: Schifano Francesco, 5469994

#include "GameOverWidget.h"                        // Include del file header del widget di Game Over
#include "Components/TextBlock.h"                  // Include per il componente di testo UTextBlock
#include "Components/Button.h"                     // Include per il componente UButton
#include "Kismet/GameplayStatics.h"                // Funzionalità di utilità per il gameplay (es. cambio livello)
#include "PAASchifanoFrancesco/Core/MyGameMode.h"  // Include del GameMode personalizzato del progetto

/**
 * Metodo: NativeConstruct
 * Descrizione: Metodo chiamato automaticamente quando il widget viene creato.
 * Associa al pulsante "ExitButton" la funzione da chiamare al click.
 */
void UGameOverWidget::NativeConstruct()
{
	Super::NativeConstruct(); // Chiama l'implementazione base per inizializzare il widget

	if (ExitButton) // Controlla che il pulsante Exit sia stato assegnato correttamente
	{
		// Collega la funzione OnClicked_Exit all'evento di click del pulsante
		ExitButton->OnClicked.AddDynamic(this, &UGameOverWidget::OnClicked_Exit);
	}
}

/**
 * Metodo: SetWinnerText
 * Descrizione: Mostra il nome del vincitore nel TextBlock WinnerText.
 * @param WinnerName - Stringa che rappresenta il nome del vincitore (es. "Player" o "AI")
 */
void UGameOverWidget::SetWinnerText(const FString& WinnerName)
{
	if (WinnerText) // Verifica che il componente testo sia valido
	{
		// Imposta il testo formattato nel widget, es: "Vittoria: Player"
		WinnerText->SetText(FText::FromString(FString::Printf(TEXT("Vittoria: %s"), *WinnerName)));
	}
}

/**
 * Metodo: OnClicked_Exit
 * Descrizione: Funzione chiamata quando il giocatore clicca il pulsante "Esci".
 * Termina l'applicazione utilizzando le librerie di sistema.
 */
void UGameOverWidget::OnClicked_Exit()
{
	// Termina il gioco e chiude la finestra, valido sia per editor che per build finale
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, true);
}