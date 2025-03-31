// Creato da: Schifano Francesco, 5469994

// Include del proprio header
#include "SelectPawn.h"

// Include delle classi di supporto necessarie per il funzionamento
#include "PAASchifanoFrancesco/Core/MyGameMode.h"
#include "PAASchifanoFrancesco/Core/PlacementManager.h"
#include "PAASchifanoFrancesco/Units/Sniper.h"
#include "PAASchifanoFrancesco/Units/Brawler.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

/**
 * Override di NativeConstruct
 * Questo metodo viene chiamato automaticamente quando il widget viene creato.
 * Viene utilizzato per inizializzare i riferimenti e fare il binding degli eventi ai pulsanti.
 */
void USelectPawn::NativeConstruct()
{
	Super::NativeConstruct(); // Chiamata al costruttore base

	GM = Cast<AMyGameMode>(UGameplayStatics::GetGameMode(this)); // Recupera il GameMode attuale
	if (GM)
	{
		PlacementManager = GM->GetPlacementManager(); // Ottiene il riferimento al PlacementManager
	}
	
	// Collega il pulsante Brawler all'evento OnBrawlerSelected
	if (BrawlerButton) BrawlerButton->OnClicked.AddDynamic(this, &USelectPawn::OnBrawlerSelected);

	// Collega il pulsante Sniper all'evento OnSniperSelected
	if (SniperButton) SniperButton->OnClicked.AddDynamic(this, &USelectPawn::OnSniperSelected);
}

/**
 * Override di NativeOnMouseButtonDown
 * Permette di intercettare il click del mouse anche al di fuori dei pulsanti.
 * In questo caso, se il tasto destro viene premuto sopra un pulsante attivo, si seleziona la pedina.
 */
FReply USelectPawn::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// Controlla se è stato premuto il tasto destro del mouse
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		// Se il mouse è sopra il pulsante Brawler → seleziona il Brawler
		if (BrawlerButton && BrawlerButton->IsHovered())
		{
			OnBrawlerSelected();
			return FReply::Handled();
		}
		// Se il mouse è sopra il pulsante Sniper → seleziona lo Sniper
		else if (SniperButton && SniperButton->IsHovered())
		{
			OnSniperSelected();
			return FReply::Handled();
		}
	}

	// Se non è stato gestito, esegue il comportamento predefinito
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

/**
 * Metodo OnBrawlerSelected
 * Seleziona il Brawler come tipo di unità da piazzare per il player.
 */
void USelectPawn::OnBrawlerSelected()
{
	PlacementManager->SetSelectedPawnType(ABrawler::StaticClass(), EPlayer::Player1);
}

/**
 * Metodo OnSniperSelected
 * Seleziona lo Sniper come tipo di unità da piazzare per il player.
 */
void USelectPawn::OnSniperSelected()
{
	PlacementManager->SetSelectedPawnType(ASniper::StaticClass(), EPlayer::Player1);
}

/**
 * Metodo DisableButtonForPawn
 * Disattiva il pulsante corrispondente all'unità appena piazzata.
 * Impedisce al giocatore di piazzare più pedine dello stesso tipo.
 */
void USelectPawn::DisableButtonForPawn(TSubclassOf<AUnitBase> PawnType)
{
	// Disattiva il pulsante Brawler se è stato selezionato
	if (PawnType == ABrawler::StaticClass() && BrawlerButton)
	{
		BrawlerButton->SetIsEnabled(false);
	}
	// Disattiva il pulsante Sniper se è stato selezionato
	else if (PawnType == ASniper::StaticClass() && SniperButton)
	{
		SniperButton->SetIsEnabled(false);
	}
}

/**
 * Metodo AreAllButtonsDisabled
 * Ritorna true se entrambi i pulsanti sono disabilitati → indica che non ci sono più unità selezionabili.
 */
bool USelectPawn::AreAllButtonsDisabled() const
{
	return (!BrawlerButton || !BrawlerButton->GetIsEnabled()) &&
		   (!SniperButton || !SniperButton->GetIsEnabled());
}