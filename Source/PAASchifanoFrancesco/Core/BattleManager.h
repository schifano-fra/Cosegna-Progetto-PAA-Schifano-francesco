

#pragma once

#include "CoreMinimal.h"
#include "PAASchifanoFrancesco/Units/UnitMovementManager.h"
#include "GameFramework/Actor.h"
#include "BattleManager.generated.h"

// Forward declaration della classe base delle unità
class AUnitBase;

// Forward declaration della classe base delle unità
class AGridManager;

// Forward declaration della classe base delle unità
class AMyGameMode;

// Forward declaration della classe base delle unità
class UTurnManager;

/*
* Classe ABattleManager
* Responsabile della gestione della fase di battaglia, incluse le azioni dell'IA, movimento, attacco
*/
UCLASS()
class PAASCHIFANOFRANCESCO_API ABattleManager : public AActor
{
	GENERATED_BODY()  // Macro che genera automaticamente il codice boilerplate richiesto da Unreal

public:
	// Costruttore
	ABattleManager();

	// Inizializza la fase di battaglia
	void StartBattle();

	// Callback chiamata al termine del movimento IA
	void OnAIMovementComplete(AUnitBase* UnitBase);

	// Inizializza il turno dell'IA
	void PrepareAITurn();

	// Prova a far attaccare un'unità IA
	bool TryAIAttack(AUnitBase* AIUnit);

	// Prova a far muovere un'unità IA verso il nemico più vicino
	void TryAIMove(AUnitBase* AIUnit);

	// Prova a far muovere un'unità IA in modo casuale
	void TryAIRandomMove(AUnitBase* AIUnit);

	// Restituisce il nemico più vicino a una determinata unità IA
	AUnitBase* FindNearestEnemy(AUnitBase* AIUnit);

private:
	// L'unità attualmente selezionata (potrebbe essere usata per interazioni specifiche)
	AUnitBase* SelectedUnit;

	// Riferimento al GridManager, responsabile della gestione delle celle della griglia
	AGridManager* GridManager;

	// Riferimento al GameMode principale del gioco
	AMyGameMode* GameMode;

	// Indice dell'unità IA attualmente in fase di elaborazione
	int32 CurrentAIIndex;

	// Lista di unità IA che devono ancora agire durante il turno corrente
	TArray<AUnitBase*> AIUnitsToProcess;

	// Gestisce la logica della prossima unità IA nel turno corrente
	void ProcessNextAIUnit();

	// Riferimento al TurnManager, gestisce i turni tra player e IA
	UPROPERTY()
	UTurnManager* TurnManager;

	// Riferimento al MovementManager globale, usato per muovere le unità
	UPROPERTY()
	AUnitMovementManager* MovementManager;
};