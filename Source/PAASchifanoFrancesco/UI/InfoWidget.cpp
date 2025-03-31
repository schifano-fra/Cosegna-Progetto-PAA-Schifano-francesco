// Creato da: Schifano Francesco, 5469994

#include "InfoWidget.h"                      // Include del file header di questo widget
#include "Components/Button.h"               // Necessario per gestire i pulsanti (UButton)
#include "Components/Image.h"                // Necessario per le immagini (UImage)
#include "Kismet/GameplayStatics.h"          // Include per funzioni globali (es. GetPlayerController)

/**
 * Metodo: NativeConstruct
 * Descrizione: Override del metodo chiamato automaticamente quando il widget è costruito e pronto all’uso.
 * Qui viene effettuato il binding del pulsante "BackButton" all’evento di click.
 */
void UInfoWidget::NativeConstruct()
{
	Super::NativeConstruct(); // Chiama il costruttore base di UUserWidget

	// Verifica che il puntatore al pulsante non sia nullo (es. che il widget sia correttamente legato dal Designer)
	if (BackButton)
	{
		// Collega l'evento OnClicked del pulsante alla funzione OnBackClicked()
		BackButton->OnClicked.AddDynamic(this, &UInfoWidget::OnBackClicked);
	}

	// Nota: le immagini FirstInformation e SecondInformation sono già posizionate via editor,
	// quindi non è necessario manipolarle via codice a runtime se restano statiche.
}

/**
 * Metodo: OnBackClicked
 * Descrizione: Gestisce il comportamento del pulsante "Back to Game".
 * Rimuove il widget dallo schermo e ripristina il controllo di gioco al Player.
 */
void UInfoWidget::OnBackClicked()
{
	// Rimuove il widget Info dalla viewport
	RemoveFromParent();

	// Recupera il controller del giocatore (indice 0)
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		// Ripristina il controllo al gameplay (esclude input da UI)
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = true; // Mantiene il cursore visibile
	}
}