#include "TurnManager.h"
#include "MyGameMode.h"
#include "BattleManager.h"
#include "PlacementManager.h"
#include "PAASchifanoFrancesco/UI/UITurnIndicator.h"
#include "PAASchifanoFrancesco/UI/StatusGameWidget.h"
#include "PAASchifanoFrancesco/Grid/GridManager.h"
#include "PAASchifanoFrancesco/Input/MyPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

/**
 *
 * Scopo del metodo:
 * Questo metodo serve per inizializzare il TurnManager, assegnandogli i riferimenti al GameMode e al BattleManager.
 * Inoltre, imposta il giocatore corrente come Player1 e collega il delegato OnCanEndTurn al widget StatusGameWidget, che
 * gestisce la visibilità del pulsante "End Turn". 
 */
void UTurnManager::Initialize(AMyGameMode* InGameMode, ABattleManager* InBattleManager)
{
    GameMode = InGameMode; // Salva il riferimento al GameMode passato come parametro
    BattleManager = InBattleManager; // Salva il riferimento al BattleManager (usato nella fase di battaglia)

    CurrentPlayer = EPlayer::Player1; // Imposta il giocatore iniziale come Player1

    if (GameMode) // Verifica che il GameMode sia valido
    {
        // Recupera il riferimento al widget che contiene il pulsante "End Turn"
        UStatusGameWidget* StatusGameWidget = GameMode->GetStatusGameWidget();

        if (StatusGameWidget) // Se il widget è stato trovato correttamente
        {
            // Collega il delegato dinamico OnCanEndTurn alla funzione ActiveButton del widget
            // Questo permette al TurnManager di mostrare/nascondere il pulsante di fine turno
            OnCanEndTurn.AddDynamic(StatusGameWidget, &UStatusGameWidget::ActiveButton);
        }
        else
        {
            // Se il widget non esiste, logga un errore per facilitare il debug
            UE_LOG(LogTemp, Error, TEXT("ERRORE: StatusGameWidget non trovato in GameMode!"));
        }
    }
}

/**
* Scopo del metodo:
* Questo metodo avvia il turno del giocatore attuale (CurrentPlayer) in base alla fase delgioco (Placement o Battle).
* Gestisce anche l’input del giocatore e il comportamento dell’IA. In fase di piazzamento, fa muovere
* automaticamente l’IA. In fase di battaglia, abilita il controllo al player o avvia l’IA.
*/
void UTurnManager::StartTurn()
{
    UpdateTurnUI(); // Aggiorna il widget grafico per mostrare a chi appartiene il turno corrente

    // Stampa nel log il nome del giocatore che sta iniziando il turno
    UE_LOG(LogTemp, Warning, TEXT("[TurnManager] Inizia il turno di: %s"), *UEnum::GetValueAsString(CurrentPlayer));

    // Controlla se siamo nella fase di piazzamento
    if (GameMode->GetCurrentGamePhase() == EGamePhase::EPlacement)
    {
        // Se è il turno dell'IA durante la fase di piazzamento
        if (CurrentPlayer == EPlayer::AI)
        {
            // Log informativo per il debug
            UE_LOG(LogTemp, Warning, TEXT("Turno dell'AI per piazzare un'unità."));

            // Recupera il PlacementManager per far piazzare una pedina all'IA
            if (APlacementManager* PM = GameMode->GetPlacementManager())
            {
                // Imposta un timer di 3 secondi prima di far piazzare la pedina all’IA
                GameMode->GetWorld()->GetTimerManager().SetTimer(AITurnTimerHandle, [this, PM]()
                {
                    PM->PlaceAIPawn(); // Piazzamento effettivo della pedina da parte dell’IA
                }, 3.0f, false);
            }
        }
    }
    // Se siamo nella fase di battaglia
    else if (GameMode->GetCurrentGamePhase() == EGamePhase::EBattle)
    {
        // Se il widget del turno è presente, nasconde il pulsante "End Turn" all'inizio del turno
        if (GameMode->GetStatusGameWidget())
        {
            GameMode->GetStatusGameWidget()->ActiveButton(false); // Il pulsante viene disattivato all’inizio del turno
        }

        // Se è il turno del player
        if (CurrentPlayer == EPlayer::Player1)
        {
            // Recupera il controller del giocatore
            if (APlayerController* PC = UGameplayStatics::GetPlayerController(GameMode, 0))
            {
                // Cast al controller personalizzato
                if (AMyPlayerController* MyPC = Cast<AMyPlayerController>(PC))
                {
                    MyPC->SetMovementLocked(false); // Sblocca il movimento per permettere l'interazione
                }
            }
        }
        // Se è il turno dell’IA e il BattleManager è disponibile
        else if (CurrentPlayer == EPlayer::AI && BattleManager)
        {
            // Log utile per tracciare l’inizio del turno dell’IA
            UE_LOG(LogTemp, Warning, TEXT("L'AI inizia il turno e controlla se può attaccare"));

            // L’IA prepara le proprie azioni (attacco o movimento) tramite il BattleManager
            BattleManager->PrepareAITurn();
        }
    }
}

/**
 * Scopo del metodo:
 * Questo metodo termina il turno del giocatore attuale e passa il turno all’altro giocatore (Player1 → AI o AI → Player1).
 * Prima di farlo, pulisce gli highlight dalla griglia e resetta lo stato di tutte le unità del giocatore uscente.
 * Dopo un breve delay, avvia il nuovo turno.
 */
void UTurnManager::EndTurn()
{
    // Log di debug per tracciare il cambio turno
    UE_LOG(LogTemp, Warning, TEXT("EndTurn() chiamato - Cambio turno in corso..."));

    // Ottiene il riferimento al GridManager per pulire la griglia
    AGridManager* GridManager = GameMode->GetGridManager();
    if (GridManager)
    {
        GridManager->ClearHighlights(); // Rimuove qualsiasi highlight (es. movimento, attacco)
    }

    // Se il turno attuale è del Player
    if (CurrentPlayer == EPlayer::Player1)
    {
        // Per ogni unità del Player, resetta lo stato di azione (Idle, Moved, Attacked)
        for (AUnitBase* PlayerUnit : GameMode->PlayerUnits)
        {
            PlayerUnit->ResetAction(); // Ritorna lo stato dell'unità a Idle
        }

        // Passa il turno all’IA
        UE_LOG(LogTemp, Warning, TEXT("Cambio turno: Ora è il turno dell'AI."));
        CurrentPlayer = EPlayer::AI;
    }
    else // Se il turno attuale è dell’IA
    {
        // Per ogni unità dell’IA, resetta lo stato di azione
        for (AUnitBase* AIUnit : GameMode->AIUnits)
        {
            AIUnit->ResetAction(); // Anche le unità IA tornano a Idle
        }

        // Passa il turno al Player
        UE_LOG(LogTemp, Warning, TEXT("Cambio turno: Ora è il turno del Player."));
        CurrentPlayer = EPlayer::Player1;
    }

    // Attende 1 secondo prima di iniziare il nuovo turno (per chiarezza visiva e transizioni)
    GameMode->GetWorld()->GetTimerManager().SetTimer( AITurnTimerHandle, this, &UTurnManager::StartTurn, 1.0f, false );
}

/**
 * Scopo del metodo:
 * Questo metodo viene chiamato esclusivamente durante la fase di piazzamento (EGamePhase::EPlacement).
 * Registra una nuova unità posizionata da un giocatore (Player o AI), la aggiunge alla lista corrispondente e gestisce
 * l'avanzamento del turno o della fase nel chiamare StartTurn().
 * @param Unit 
 */
void UTurnManager::RegisterPlacementMove(AUnitBase* Unit)
{
    // Se l'unità è stata piazzata dal Player, la aggiungiamo alla lista PlayerUnits
    if (CurrentPlayer == EPlayer::Player1)
    {
        GameMode->PlayerUnits.Add(Unit);  // Salva la nuova unità del player
    }
    else // Altrimenti è dell’IA
    {
        GameMode->AIUnits.Add(Unit);  // Salva la nuova unità dell'IA
    }

    // Verifica se entrambe le fazioni hanno piazzato almeno due unità
    if (GameMode->PlayerUnits.Num() > 1 && GameMode->AIUnits.Num() > 1)
    {
        // Se sì, allora si può terminare la fase di piazzamento e passare alla battaglia
        if (GameMode && GameMode->GetPlacementManager())
        {
            // Chiama il metodo per terminare la fase di piazzamento
            GameMode->GetPlacementManager()->FinishPlacementPhase();
        }
    }
    else
    {
        // Se non tutte le unità sono ancora state piazzate, si passa il turno all’altro giocatore
        CurrentPlayer = (CurrentPlayer == EPlayer::Player1) ? EPlayer::AI : EPlayer::Player1;

        // Si avvia un nuovo turno di piazzamento (AI o Player in base al cambio sopra)
        StartTurn();
    }
}

// Questi metodi vengono chiamati esclusivamente durante la fase di battaglia (EGamePhase::EBattle),
// per registrare le azioni effettuate da ogni unità (player o IA) e tenere traccia dello stato del turno.


/**
 * Scopo del metodo:
 * Chiamato quando una unità del Player effettua un movimento durante la fase di battaglia.
 * Imposta lo stato dell’unità su Moved e poi controlla se il turno può finire.
 * @param Unit 
 */
void UTurnManager::RegisterPlayerMove(AUnitBase* Unit)
{
    Unit->SetCurrentAction(EUnitAction::Moved); // Imposta lo stato dell'unità su "Moved"
    CheckPlayerEndTurn(Unit); // Verifica se è possibile terminare il turno del player
}

/**
 * Scopo del metodo:
 * Chiamato quando una unità del Player effettua un attacco nella fase di battaglia.
 * Imposta lo stato su Attacked e controlla se il turno può terminare.
 * @param Unit 
 */
void UTurnManager::RegisterPlayerAttack(AUnitBase* Unit)
{
    Unit->SetCurrentAction(EUnitAction::Attacked);
    CheckPlayerEndTurn(Unit);
}

/**
 * Scopo del metodo:
 * Chiamato quando una unità dell’IA si muove durante il proprio turno nella fase di battaglia.
 * Imposta semplicemente lo stato dell’unità su Moved.
 * @param Unit 
 */
void UTurnManager::RegisterAIMove(AUnitBase* Unit)
{
    Unit->SetCurrentAction(EUnitAction::Moved);
}

/**
 * Scopo del metodo:
 * Chiamato quando una unità dell’IA attacca durante il proprio turno nella fase di battaglia.
 * Imposta lo stato su Attacked.
 * @param Unit 
 */
void UTurnManager::RegisterAIAttack(AUnitBase* Unit)
{
    Unit->SetCurrentAction(EUnitAction::Attacked);
}

/**
 * Descrizione:
 * Questo metodo imposta quale giocatore (Player o AI) inizia la partita.
 * Viene utilizzato dopo la fase del lancio della moneta (Coin Flip) o in situazioni
 * in cui si vuole forzare l’ordine dei turni manualmente.
 *
 * @param StartingPlayer 
 */
void UTurnManager::SetInitialPlayer(EPlayer StartingPlayer)
{
    CurrentPlayer = StartingPlayer;
}

/**
 * Descrizione:
 * Aggiorna l’interfaccia utente per indicare visivamente quale giocatore ha il turno attivo.
 * Viene chiamato all'inizio di ogni turno tramite il metodo `StartTurn()`.
 *
 * Funzionamento:
 * - Recupera il nome del giocatore attuale (Player o AI)
 * - Passa il nome al widget `TurnIndicator` per aggiornare il testo mostrato a schermo
 */
void UTurnManager::UpdateTurnUI()
{
    // Verifica che GameMode e il widget TurnIndicator siano validi
    if (GameMode && GameMode->GetTurnIndicatorWidget())
    {
        // Determina il nome da mostrare a seconda del giocatore corrente
        FString PlayerName = (CurrentPlayer == EPlayer::Player1) ? TEXT("Player") : TEXT("AI");

        // Aggiorna il testo nel widget TurnIndicator
        GameMode->GetTurnIndicatorWidget()->UpdateTurnText(PlayerName);
    }
}

/**
 * Descrizione:
 * Questo metodo viene utilizzato **durante la fase di battaglia** (EGamePhase::EBattle) per verificare se il
 * giocatore umano ha completato tutte le azioni delle proprie unità.
 * È chiamato ogni volta che un'unità effettua un'azione (movimento o attacco).
 *
 * Il turno del player può terminare solo quando:
 * - Tutte le unità hanno agito (nessuna è nello stato Idle).
 * - Se almeno una unità ha solo mosso (stato "Moved") e nessuna è più in "Idle", viene attivato il pulsante "End Turn"
 *   per permettere al giocatore di decidere.
 * - Se tutte le unità sono in stato "Attacked", il turno finisce automaticamente(non c’è più nulla da fare).
 *
 * Questa logica consente al giocatore:
 * - Di muovere una o più unità e decidere se attaccare o passare il turno.
 * - Di non essere forzato a terminare subito il turno solo perché ha mosso tutte le unità.
 *
 * @param Unit 
 */
void UTurnManager::CheckPlayerEndTurn(AUnitBase* Unit)
{
    // Verifica che GameMode sia valido
    if (!GameMode) return;

    // Se l'unità passata non è valida o non è controllata dal player, esci
    if (!Unit || !Unit->IsPlayerControlled()) return;

    // Flag per capire se ci sono ancora unità Idle o se almeno una ha solo mosso
    bool bHasMovedUnit = false;  // Almeno una unità è in stato "Moved"
    bool bHasIdleUnit = false;   // Almeno una unità è ancora "Idle"

    // Itera su tutte le unità del giocatore
    for (AUnitBase* PlayerUnit : GameMode->PlayerUnits)
    {
        // Ottiene lo stato corrente dell’unità
        EUnitAction Action = PlayerUnit->GetCurrentAction();

        // Caso 1: c'è ancora un'unità che non ha fatto nulla
        if (Action == EUnitAction::Idle)
        {
            // Log utile per debugging
            UE_LOG(LogTemp, Warning, TEXT("⏳ Unità %s è ancora in Idle. Il turno non può finire."), *PlayerUnit->GetName());

            bHasIdleUnit = true;  // Blocca la possibilità di terminare il turno
            break;  // Non serve continuare, una sola Idle è sufficiente
        }

        // Caso 2: almeno una unità ha solo mosso (e potrebbe attaccare)
        if (Action == EUnitAction::Moved)
        {
            bHasMovedUnit = true;  // Serve per decidere se mostrare il pulsante
        }
    }

    // Se anche una sola unità è in Idle, non è ancora possibile finire il turno
    if (bHasIdleUnit)
    {
        return;
    }

    // Caso 3: Nessuna unità è più Idle, e almeno una ha solo mosso
    if (bHasMovedUnit)
    {
        // Mostra il pulsante "Fine Turno" → il giocatore può decidere se attaccare o terminare
        UE_LOG(LogTemp, Warning, TEXT("Nessuna unità è Idle, e almeno una ha solo mosso. Mostro pulsante di fine turno."));
        OnCanEndTurn.Broadcast(true);  // Attiva il pulsante nella UI
    }
    else
    {
        // Caso 4: tutte le unità sono in stato "Attacked" → nessuna azione residua
        UE_LOG(LogTemp, Warning, TEXT("Nessuna unità è Idle o Moved. Tutte hanno attaccato. Fine turno automatica."));
        EndTurn();  // Chiama direttamente la fine del turno
    }
}