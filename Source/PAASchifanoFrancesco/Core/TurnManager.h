// Creato da: Schifano Francesco, 5469994

#pragma once

#include "CoreMinimal.h"
#include "MyGameMode.h"
#include "UObject/NoExportTypes.h"
#include "TurnManager.generated.h"

// Forward declarations per evitare dipendenze circolari
class ABattleManager;
class AUnitBase;

/**
 * Delegato dinamico utilizzato per notificare al sistema UI
 * se il pulsante "Fine Turno" deve essere visibile o meno.
 * Parametro: bool IsVisible → indica se il bottone deve essere mostrato.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCanEndTurn, bool, IsVisible);

/**
 * Classe: UTurnManager
 * Descrizione: gestisce il flusso dei turni nel gioco a turni.
 * Responsabile dell'inizio e fine turno, del tracciamento delle azioni effettuate
 * da parte del player e dell'IA, e della comunicazione con GameMode e BattleManager.
 */
UCLASS()
class PAASCHIFANOFRANCESCO_API UTurnManager : public UObject
{
    GENERATED_BODY()

public:

    /**
     * Metodo: Initialize
     * Inizializza il TurnManager con il GameMode e opzionalmente con il BattleManager.
     * Va chiamato all'avvio della fase CoinFlip o Battle.
     */
    void Initialize(AMyGameMode* InGameMode, ABattleManager* InBattleManager = nullptr);

    /**
     * Metodo: StartTurn
     * Avvia un nuovo turno per il giocatore corrente.
     * Notifica le unità, aggiorna la UI, e gestisce turni automatici dell’IA.
     */
    void StartTurn();

    /**
     * Metodo: EndTurn
     * Termina il turno del giocatore corrente e passa al successivo.
     * Aggiorna la UI e chiama nuovamente StartTurn.
     */
    void EndTurn();

    /**
     * Metodo: RegisterPlayerMove
     * Registra che una specifica unità del player ha effettuato un movimento.
     */
    void RegisterPlayerMove(AUnitBase* Unit);

    /**
     * Metodo: RegisterPlayerAttack
     * Registra che una specifica unità del player ha effettuato un attacco.
     */
    void RegisterPlayerAttack(AUnitBase* Unit);

    /**
     * Metodo: RegisterAIMove
     * Registra che una specifica unità dell'IA ha effettuato un movimento.
     * Utile per tracciamento interno.
     */
    void RegisterAIMove(AUnitBase* Unit);

    /**
     * Metodo: RegisterAIAttack
     * Registra che una specifica unità dell'IA ha effettuato un attacco.
     */
    void RegisterAIAttack(AUnitBase* Unit);

    /**
     * Metodo: RegisterPlacementMove
     * Registra che una unità è stata piazzata durante la fase di piazzamento.
     * Utile per distinguere tra placement e azioni da battaglia.
     */
    void RegisterPlacementMove(AUnitBase* Unit);

    /**
     * Metodo: UpdateTurnUI
     * Aggiorna gli elementi UI legati al turno corrente, come indicatori visivi o testo.
     */
    void UpdateTurnUI();

    /**
     * Metodo: CheckPlayerEndTurn
     * Verifica se tutte le unità del player hanno agito.
     * In tal caso, abilita il pulsante per terminare il turno.
     */
    void CheckPlayerEndTurn(AUnitBase* Unit);

    /**
     * Metodo: SetInitialPlayer
     * Imposta quale giocatore inizia la partita (usato dopo il CoinFlip).
     */
    void SetInitialPlayer(EPlayer StartingPlayer);

    /**
     * Metodo Getter: GetCurrentPlayer
     * Ritorna il giocatore che sta attualmente svolgendo il turno.
     */
    EPlayer GetCurrentPlayer() const { return CurrentPlayer; }

private:

    /** Riferimento al GameMode principale per accedere a unità e fase attuale */
    UPROPERTY()
    AMyGameMode* GameMode;

    /** Riferimento al BattleManager per controllare e avanzare azioni IA */
    UPROPERTY()
    ABattleManager* BattleManager;

    /** Giocatore corrente (Player1 o AI) */
    UPROPERTY()
    EPlayer CurrentPlayer;

    /** Delegato per la UI che segnala se è possibile terminare il turno */
    UPROPERTY()
    FOnCanEndTurn OnCanEndTurn;

    /** Timer per gestire il delay dei turni automatici dell’IA */
    FTimerHandle AITurnTimerHandle;
};
