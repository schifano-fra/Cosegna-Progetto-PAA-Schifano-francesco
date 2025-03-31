#include "StatusGameWidget.h" // Include dell'header della classe
#include "PAASchifanoFrancesco/Core/MyGameMode.h" // Per accedere al GameMode
#include "PAASchifanoFrancesco/Core/TurnManager.h" // Per accedere al TurnManager
#include "Components/Border.h"         // Widget contenitore grafico
#include "Components/Button.h"         // Per il pulsante "End Turn"
#include "Components/TextBlock.h"      // Per il nome dell'unità
#include "Components/VerticalBox.h"    // Contenitore per gli elementi
#include "Components/ProgressBar.h"    // Barra della vita
#include "Components/SizeBox.h"        // Per settare dimensioni fisse
#include "Components/VerticalBoxSlot.h"// Slot per la disposizione verticale
#include "Kismet/GameplayStatics.h"    // Utility per ottenere il GameMode

/**
 * Metodo chiamato automaticamente quando il widget è inizializzato.
 * Configura il pulsante di fine turno.
 */
void UStatusGameWidget::NativeConstruct()
{
	Super::NativeConstruct(); // Chiama implementazione base

	if (EndButton) // Verifica che il pulsante sia stato correttamente assegnato via Designer
	{
		EndButton->SetIsEnabled(false); // Disabilita inizialmente
		EndButton->SetVisibility(ESlateVisibility::Hidden); // Nasconde
		EndButton->OnClicked.AddDynamic(this, &UStatusGameWidget::OnClickedEndTurn); // Collega evento
	}
}

/**
 * Metodo chiamato al clic del pulsante "End Turn".
 * Chiama il TurnManager per terminare il turno corrente.
 */
void UStatusGameWidget::OnClickedEndTurn()
{
	// Recupera il GameMode
	if (AMyGameMode* GM = Cast<AMyGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		// Recupera il TurnManager
		if (UTurnManager* TurnManager = GM->GetTurnManager())
		{
			TurnManager->EndTurn(); // Termina il turno

			// Disattiva e nasconde il pulsante
			EndButton->SetVisibility(ESlateVisibility::Hidden);
			EndButton->SetIsEnabled(false);
		}
	}
}

/**
 * Metodo che attiva o disattiva visibilità e interattività del pulsante.
 * @param bIsVisible - true se deve essere visibile e cliccabile.
 */
void UStatusGameWidget::ActiveButton(bool bIsVisible)
{
	if (EndButton)
	{
		EndButton->SetIsEnabled(bIsVisible);
		EndButton->SetVisibility(bIsVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

/**
 * Metodo che aggiunge graficamente una barra della vita per una nuova unità.
 * @param Unit - puntatore all'unità da visualizzare.
 */
void UStatusGameWidget::AddUnitStatus(AUnitBase* Unit)
{
	// Verifica validità dell'unità e che non sia già presente
	if (!Unit || !UnitStatusBox || UnitHealthBars.Contains(Unit)) return;

	// Crea contenitore con sfondo bianco
	UBorder* BackgroundBox = NewObject<UBorder>(this);
	BackgroundBox->SetBrushColor(FLinearColor::White); // Colore sfondo
	BackgroundBox->SetPadding(FMargin(10.f));
	BackgroundBox->SetHorizontalAlignment(HAlign_Fill);

	// Crea contenitore interno verticale
	UVerticalBox* InnerBox = NewObject<UVerticalBox>(this);

	// Crea il blocco di testo con il nome dell’unità
	UTextBlock* NameText = NewObject<UTextBlock>(this);
	NameText->SetText(FText::FromString(Unit->UnitDisplayName));
	FSlateFontInfo FontInfo = NameText->GetFont();
	FontInfo.Size = 24; // Dimensione font
	NameText->SetFont(FontInfo);
	NameText->SetJustification(ETextJustify::Left);

	// Colore in base al tipo di unità
	NameText->SetColorAndOpacity(Unit->IsPlayerControlled() ? FSlateColor(FColor::Blue) : FSlateColor(FColor(139, 69, 19)));

	// Aggiungi il nome alla box interna
	UVerticalBoxSlot* NameSlot = InnerBox->AddChildToVerticalBox(NameText);
	NameSlot->SetPadding(FMargin(5.f, 5.f, 5.f, 2.f));
	NameSlot->SetHorizontalAlignment(HAlign_Left);

	// Crea la barra della vita inizialmente piena (1.0)
	UProgressBar* HealthBar = NewObject<UProgressBar>(this);
	HealthBar->SetPercent(1.0f); // Salute iniziale
	HealthBar->SetFillColorAndOpacity(FLinearColor::Green); // Colore verde

	// Contenitore per la barra con larghezza/altezza fissa
	USizeBox* SizeBox = NewObject<USizeBox>(this);
	SizeBox->SetWidthOverride(300.f);
	SizeBox->SetHeightOverride(25.f);
	SizeBox->AddChild(HealthBar);

	// Aggiunge barra alla box verticale
	UVerticalBoxSlot* BarSlot = InnerBox->AddChildToVerticalBox(SizeBox);
	BarSlot->SetPadding(FMargin(5.f, 2.f, 5.f, 5.f));
	BarSlot->SetHorizontalAlignment(HAlign_Left);

	// Imposta la box interna come contenuto del contenitore esterno
	BackgroundBox->SetContent(InnerBox);

	// Aggiunge l'intero contenitore all'interfaccia grafica
	UVerticalBoxSlot* ContainerSlot = Cast<UVerticalBoxSlot>(UnitStatusBox->AddChild(BackgroundBox));
	if (ContainerSlot)
	{
		ContainerSlot->SetPadding(FMargin(10.f, 5.f, 10.f, 5.f));
		ContainerSlot->SetHorizontalAlignment(HAlign_Left);
	}

	// Salva nella mappa il riferimento alla barra e al contenitore per aggiornamenti futuri
	UnitHealthBars.Add(Unit, TPair<UProgressBar*, UBorder*>(HealthBar, BackgroundBox));
}

/**
 * Metodo che aggiorna graficamente la vita di un'unità.
 * @param Unit - l’unità da aggiornare.
 * @param NewHealthPercent - percentuale di vita attuale (tra 0.0 e 1.0)
 */
void UStatusGameWidget::UpdateUnitHealth(AUnitBase* Unit, float NewHealthPercent)
{
	if (!Unit || !IsValid(Unit))
	{
		UE_LOG(LogTemp, Error, TEXT("UpdateUnitHealth: Unità nulla o non valida"));
		return;
	}

	if (!UnitHealthBars.Contains(Unit))
	{
		UE_LOG(LogTemp, Warning, TEXT("UpdateUnitHealth: Nessuna barra trovata per %s"), *Unit->GetName());
		return;
	}

	TPair<UProgressBar*, UBorder*>* FoundPair = UnitHealthBars.Find(Unit);
	if (!FoundPair)
	{
		UE_LOG(LogTemp, Error, TEXT("UpdateUnitHealth: Pair non trovato per %s"), *Unit->GetName());
		return;
	}

	UProgressBar* Bar = FoundPair->Key;
	if (!Bar)
	{
		UE_LOG(LogTemp, Error, TEXT("UpdateUnitHealth: Barra NULL per %s"), *Unit->GetName());
		return;
	}

	// Imposta la percentuale di salute
	Bar->SetPercent(NewHealthPercent);

	// Colore dinamico in base alla percentuale
	if (NewHealthPercent < 0.3f)
	{
		Bar->SetFillColorAndOpacity(FLinearColor::Red); // Bassa vita
	}
	else if (NewHealthPercent < 0.9f)
	{
		Bar->SetFillColorAndOpacity(FLinearColor::Yellow); // Vita media
	}
	else
	{
		Bar->SetFillColorAndOpacity(FLinearColor::Green); // Alta vita
	}
	Bar->InvalidateLayoutAndVolatility();
}

/**
 * Metodo che rimuove l’elemento UI di un’unità morta dalla schermata.
 * @param Unit - unità da rimuovere
 */
void UStatusGameWidget::RemoveUnitStatus(AUnitBase* Unit)
{
	if (!UnitStatusBox || !UnitHealthBars.Contains(Unit)) return;

	TPair<UProgressBar*, UBorder*> Pair = UnitHealthBars[Unit];
	UBorder* Container = Pair.Value;

	if (Container && Container->IsValidLowLevel())
	{
		UnitStatusBox->RemoveChild(Container);
	}

	UnitHealthBars.Remove(Unit); // Rimuove dal dizionario
}