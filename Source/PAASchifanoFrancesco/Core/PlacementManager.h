// Creato da: Schifano Francesco, 5469994

#pragma once

#include "CoreMinimal.h"
#include "MyGameMode.h"
#include "GameFramework/Actor.h"
#include "PlacementManager.generated.h"

// Forward Declarations
class AGridManager;
class ATile;
class USelectPawn;
class AUnitBase;

// Delegato per notificare la fine della fase di piazzamento
DECLARE_MULTICAST_DELEGATE(FOnPlacementCompleted);

/**
 * Classe responsabile della gestione della fase di piazzamento delle unità.
 * Gestisce il widget di selezione, la scelta delle unità, il piazzamento del giocatore e dell'IA.
 */
UCLASS()
class PAASCHIFANOFRANCESCO_API APlacementManager : public AActor
{
	GENERATED_BODY()

public:

	/** Costruttore: disabilita il tick automatico e inizializza puntatori */
	APlacementManager();

	/** Inizializza la classe del widget per la selezione delle unità e il GameMode */
	void Initialize(TSubclassOf<USelectPawn> InSelectPawnClass);

	/** Avvia la fase di piazzamento: mostra il widget, prepara la griglia e inizia il turno */
	void StartPlacement();

	/** Imposta il tipo di pedina selezionata da piazzare per Player o AI */
	void SetSelectedPawnType(TSubclassOf<AUnitBase> PawnType, EPlayer Player);

	/** Gestisce il click su una tile da parte del giocatore durante il piazzamento */
	void HandleTileClick(ATile* ClickedTile);

	/** Termina la fase di piazzamento e passa alla fase di battaglia */
	void FinishPlacementPhase();

	/** Posiziona automaticamente una pedina dell'IA su una tile disponibile */
	void PlaceAIPawn();

	/** Delegato per notificare altri oggetti (es. GameMode) della fine del piazzamento */
	FOnPlacementCompleted OnPlacementCompleted;

private:

	// Riferimento al GameMode principale
	AMyGameMode* GM;

	// Riferimento alla griglia per recuperare le tile disponibili
	AGridManager* GridManager;

	// Riferimento al widget di selezione delle pedine
	USelectPawn* SelectPawn;

	// Tipo di unità da piazzare per il giocatore
	TSubclassOf<AUnitBase> PlayerPawnType;

	// Tipo di unità da piazzare per l'IA
	TSubclassOf<AUnitBase> AIPawnType;

	// Classe del widget USelectPawn da istanziare
	UPROPERTY()
	TSubclassOf<USelectPawn> SelectPawnClass;

	/** Metodo interno per piazzare un'unità del giocatore sulla tile selezionata */
	void PlacePlayerPawn(ATile* ClickedTile);
};
