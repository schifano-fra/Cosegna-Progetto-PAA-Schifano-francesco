// Creato da: Schifano Francesco 5469994

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UnitBase.h"
#include "UnitMovementManager.generated.h"

/**
 * Delegato dinamico per notificare che un'unità ha completato il proprio movimento.
 * Viene utilizzato per segnalare il termine del movimento a chi ascolta (es. PlayerController o BattleManager).
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnitMovementFinished, AUnitBase*, MovedUnit);

/**
 * Descrizione generale:
 * Questa classe è responsabile della gestione del movimento delle unità sul campo di gioco.
 * Si occupa di:
 * - Eseguire il movimento delle unità lungo un percorso specificato (array di celle ATile*)
 * - Aggiornare lo stato delle celle di partenza e arrivo
 * - Notificare tramite delegato il completamento del movimento
 */
UCLASS()
class PAASCHIFANOFRANCESCO_API AUnitMovementManager : public AActor
{
	GENERATED_BODY()

public:

	/** Costruttore di default */
	AUnitMovementManager();

	/**
	 * Delegate che viene attivato alla fine del movimento di un'unità.
	 * Utilizzato per informare GameMode, PlayerController o altri manager.
	 */
	UPROPERTY()
	FOnUnitMovementFinished OnMovementFinished;

	/**
	 * Metodo principale per avviare il movimento di una unità lungo un percorso definito.
	 * 
	 * @param Unit - L'unità da muovere
	 * @param Path - Il percorso da seguire, composto da celle (ATile*)
	 * @param Speed - La velocità del movimento
	 */
	void MoveUnit(AUnitBase* Unit, const TArray<ATile*>& Path, float Speed);

private:

	/**
	 * Callback privata chiamata automaticamente quando il movimento termina.
	 * Emette il delegato OnMovementFinished e pulisce lo stato interno.
	 */
	UFUNCTION()
	void OnUnitMovementComplete();

	/**
	 * Metodo interno che restituisce true se un'unità è attualmente in movimento.
	 * Utile per bloccare doppi movimenti o comportamenti errati.
	 */
	bool IsMovementInProgress() const;

	/** Puntatore all'unità che si sta muovendo attualmente (nullptr se nessuna) */
	AUnitBase* CurrentMovingUnit;
};
