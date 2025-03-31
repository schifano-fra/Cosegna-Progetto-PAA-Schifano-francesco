#include "UITurnIndicator.h"
#include "InfoWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Kismet/GameplayStatics.h"

void UUITurnIndicator::NativeConstruct()
{
	Super::NativeConstruct(); // Chiama la versione base per inizializzare il widget

	// Imposta il testo iniziale del turno
	if (TurnText)
	{
		TurnText->SetText(FText::FromString("Inizio Partita"));
	}

	// Pulisce la cronologia dei turni
	if (HistoryBox)
	{
		HistoryBox->ClearChildren();
	}

	// Collega il pulsante Info al suo handler
	if (InfoButton)
	{
		InfoButton->OnClicked.AddDynamic(this, &UUITurnIndicator::OnInfoButtonClicked);
	}
}

void UUITurnIndicator::UpdateTurnText(FString PlayerName)
{
	if (TurnText)
	{
		TurnText->SetText(FText::FromString("Turno di: " + PlayerName));
	}
}

void UUITurnIndicator::AddHistoryEntry(const FString& Entry)
{
	if (!HistoryBox) return; // Controllo sicurezza

	// Crea un nuovo blocco di testo per la riga di storico
	UTextBlock* NewText = NewObject<UTextBlock>(this, UTextBlock::StaticClass());
	if (NewText)
	{
		NewText->SetText(FText::FromString(Entry)); // Imposta il contenuto
		NewText->Font.Size = 16;                    // Font di dimensione leggibile
		HistoryBox->AddChildToVerticalBox(NewText); // Aggiunge alla VerticalBox
	}
}

void UUITurnIndicator::OnInfoButtonClicked()
{
	if (!InfoWidgetClass) return; // Verifica che il widget sia stato assegnato

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return; // Sicurezza

	// Crea il widget se non è già stato creato
	if (!InfoWidgetInstance)
	{
		InfoWidgetInstance = CreateWidget<UInfoWidget>(PC, InfoWidgetClass);
	}

	// Aggiunge il widget alla viewport se non già visibile
	if (InfoWidgetInstance && !InfoWidgetInstance->IsInViewport())
	{
		InfoWidgetInstance->AddToViewport();
		PC->SetInputMode(FInputModeUIOnly());     // Passa al controllo UI
		PC->bShowMouseCursor = true;              // Mostra il cursore per interazione
	}
}