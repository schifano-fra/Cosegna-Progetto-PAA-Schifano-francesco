// Creato da: Schifano Francesco 5469994

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tile.h"
#include "PAASchifanoFrancesco/Units/UnitBase.h"
#include "GridManager.generated.h"

// Forward declaration per evitare include inutili
class UTurnManager;
class AUnitBase;

/**
 * Descrizione:
 * Questa classe gestisce la generazione, la logica e le interazioni della griglia del gioco.
 * La griglia è composta da attori di tipo ATile. Il manager permette:
 * - la generazione della griglia e degli ostacoli
 * - la visualizzazione delle celle di movimento e attacco
 * - la selezione delle tile per spostamenti e combattimenti
 * - il calcolo dei percorsi per il movimento
 */
UCLASS()
class PAASCHIFANOFRANCESCO_API AGridManager : public AActor
{
	GENERATED_BODY()

public:

	// Costruttore
	AGridManager();
	
	virtual void Tick(float DeltaTime) override;

	// Genera la griglia rettangolare con dimensioni e tile specificate
	void GenerateGrid();

	// Genera ostacoli casuali mantenendo accessibilità
	void GenerateObstacles();

	// Restituisce l'intera griglia
	const TArray<ATile*>& GetGridTiles() const { return Grid; }

	// Calcola le tile raggiungibili per una data unità
	TArray<ATile*> GetValidMovementTiles(AUnitBase* SelectedUnit);

	// Evidenzia le tile raggiungibili per una unità (in blu)
	void HighlightMovementTiles(AUnitBase* SelectedUnit);

	// Rimuove ogni evidenziazione (attacco o movimento)
	void ClearHighlights();

	// Restituisce la tile alla posizione fornita (X,Y)
	ATile* FindTileAtLocation(FVector Location);

	// Restituisce le tile d'attacco valide per un'unità
	TArray<ATile*> GetValidAttackTiles(AUnitBase* Attacker);

	// Evidenzia la tile sotto l'unità selezionata (in arancione o altro colore)
	void HighlightTileUnderUnit(AUnitBase* Unit, const FLinearColor& Color);

	// Riferimento alle tile attualmente evidenziate
	UPROPERTY()
	TArray<ATile*> HighlightedTiles;

	// Imposta posizione finale dell'unità e aggiorna tile occupate
	void FinalizeUnitMovement(AUnitBase* Unit, ATile* DestinationTile);

	// Evidenzia tutte le tile dove un'unità può attaccare
	void HighlightAttackGrid(AUnitBase* AttackingUnit);

	// Restituisce eventuale unità presente su una tile
	AUnitBase* GetUnitOnTile(ATile* Tile) const;

	// Calcola un percorso tra due tile (BFS)
	UFUNCTION()
	TArray<ATile*> GetPathToTile(AUnitBase* Unit, ATile* Destination);

	// Riferimento al GameMode per accedere a TurnManager e altro
	UPROPERTY()
	class AMyGameMode* GameMode;

protected:
	// Chiamato a inizio gioco
	virtual void BeginPlay() override;

private:

	// Dimensione della griglia in X (colonne)
	UPROPERTY(EditAnywhere, Category = "Grid")
	int DimGridX = 25;

	// Dimensione della griglia in Y (righe)
	UPROPERTY(EditAnywhere, Category = "Grid")
	int DimGridY = 25;

	// Dimensione di ciascuna cella in Unreal units
	UPROPERTY(EditAnywhere, Category = "Grid")
	float CellSize = 100.0f;

	// Spaziatura tra le celle
	UPROPERTY(EditAnywhere, Category = "Grid")
	float Spacing = 10.0f;

	// Percentuale di ostacoli sulla griglia
	UPROPERTY(EditAnywhere, Category = "Grid")
	float ObstaclePercentage = 0.3f;

	// Classe di tile da spawnare
	UPROPERTY(EditAnywhere, Category = "Grid")
	TSubclassOf<ATile> TileClass;

	// Tutte le tile generate
	TArray<ATile*> Grid;

	// Algoritmo DFS per garantire accessibilità tra le tile
	void DFS(ATile* CurrentTile, TSet<ATile*>& Visited, int32 MaxObstacles);

	// Restituisce le tile adiacenti ad una data tile
	TArray<ATile*> GetNeighbors(ATile* Tile);

	// Riferimento alla tile sotto l’unità selezionata
	UPROPERTY()
	ATile* TileUnderSelectedUnit = nullptr;

	// Riferimento al TurnManager per accedere al turno corrente
	UPROPERTY()
	UTurnManager* TurnManager;

	// Indica se la griglia d'attacco è attualmente visibile
	bool bAttackGridVisible = false;

	// Lista delle tile nella griglia d’attacco
	UPROPERTY()
	TArray<ATile*> AttackGridTiles;
};