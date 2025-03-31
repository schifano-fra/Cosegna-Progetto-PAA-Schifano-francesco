//Creato da: Schifano Francesco 5469994

#include "MyPlayerController.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "PAASchifanoFrancesco/Grid/Tile.h"
#include "PAASchifanoFrancesco/Units/UnitBase.h"
#include "PAASchifanoFrancesco/Core/MyGameMode.h"
#include "PAASchifanoFrancesco/Core/PlacementManager.h"
#include "PAASchifanoFrancesco/Units/UnitMovementManager.h"
#include "PAASchifanoFrancesco/Core/TurnManager.h"
#include "Engine/DamageEvents.h"

/**
 * Costruttore della classe AMyPlayerController
 * Inizializza i riferimenti principali usati durante l'interazione del giocatore.
 * SelectedUnit viene inizializzato a nullptr (nessuna unità selezionata inizialmente).
 * bIsGridLocked è false: il giocatore può interagire con la griglia al caricamento iniziale.
 */
AMyPlayerController::AMyPlayerController()
{
    SelectedUnit = nullptr;       // Nessuna unità selezionata all'inizio
    bIsGridLocked = false;        // La griglia non è bloccata, il giocatore può interagire
}

/**
 * Chiamato automaticamente all'inizio del gioco.
 * Recupera il GameMode attivo e inizializza il GridManager e MovementManager.
 * Attiva la possibilità di rilevare eventi di click e di mouse-over.
 * Collega il metodo OnUnitMovementFinished al delegato del MovementManager,
 * così il controller verrà notificato al termine di ogni movimento.
 */
void AMyPlayerController::BeginPlay()
{
    Super::BeginPlay(); // Chiama la versione base per inizializzare correttamente l'Actor

    // Ottiene il riferimento al GameMode principale del gioco
    GameMode = Cast<AMyGameMode>(UGameplayStatics::GetGameMode(this));

    // Abilita gli eventi di click e di passaggio del mouse sugli oggetti
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;

    // Verifica che GameMode e MovementManager siano validi
    if (GameMode && GameMode->GlobalMovementManager)
    {
        // Ottiene il riferimento al gestore della griglia
        GridManager = GameMode->GetGridManager();

        // Ottiene il MovementManager globale (usato per gestire i movimenti delle unità)
        MovementManager = GameMode->GlobalMovementManager;

        // Collega il metodo di callback da eseguire quando un movimento unità è completato
        MovementManager->OnMovementFinished.AddDynamic(this, &AMyPlayerController::OnUnitMovementFinished);
    }
    else
    {
        // Se manca uno dei riferimenti fondamentali, il gioco non può proseguire correttamente
        UE_LOG(LogTemp, Fatal, TEXT("MovementManager o GameMode mancante."));
    }
}

/**
 * Collega gli input personalizzati del giocatore alle relative funzioni.
 * In particolare, associa il click sinistro (`LeftClick`) al metodo `OnLeftClick`
 * e il click destro (`RightClick`) al metodo `OnRightClick`.
 * Questi eventi sono stati mappati in Project Settings > Input.
 */
void AMyPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent(); // Inizializza i componenti di input ereditati

    if (!InputComponent) return; // Se per qualche motivo il componente input non esiste, esce

    // Collega il click sinistro al metodo che gestisce l'interazione primaria
    InputComponent->BindAction("LeftClick", IE_Pressed, this, &AMyPlayerController::OnLeftClick);

    // Collega il click destro al metodo che gestisce le azioni secondarie (come attacco)
    InputComponent->BindAction("RightClick", IE_Pressed, this, &AMyPlayerController::OnRightClick);
}


/** 
 * Questo metodo viene eseguito ogni volta che il giocatore clicca con il tasto sinistro del mouse.
 * In base alla fase di gioco (piazzamento o battaglia), il metodo delega la gestione
 * del click ad una funzione specifica: HandlePlacementClick o HandleBattleClick.
 */
void AMyPlayerController::OnLeftClick()
{
    // Imposta la modalità di input su "GameOnly" e permette al click di non essere bloccato dalla UI
    FInputModeGameOnly InputMode;
    InputMode.SetConsumeCaptureMouseDown(false); // Permette che il click passi anche attraverso i widget
    SetInputMode(InputMode); // Applica la modalità di input

    // Se la griglia è bloccata (es. in animazione), oppure GameMode o TurnManager non sono validi, esce
    if (bIsGridLocked || !GameMode || !GameMode->TurnManager) return;

    // Se GridManager non è ancora stato inizializzato, lo ottiene dal GameMode
    if (!GridManager) GridManager = GameMode->GetGridManager();
    if (!GridManager) return; // Se non esiste nemmeno dopo, esce

    // Esegue un trace sotto il cursore del mouse per capire su cosa è stato cliccato
    FHitResult Hit;
    if (!GetHitResultUnderCursor(ECC_Visibility, false, Hit) || !Hit.GetActor()) return;

    // Ottiene l'attore colpito dal click
    AActor* HitActor = Hit.GetActor();

    // Controlla in quale fase del gioco ci troviamo
    switch (GameMode->GetCurrentGamePhase())
    {
    case EGamePhase::EPlacement:
        // Se siamo nella fase di piazzamento, gestisce il click come selezione di una tile per il piazzamento
            HandlePlacementClick(HitActor);
        break;

    case EGamePhase::EBattle:
        // Se siamo nella fase di battaglia, delega la gestione del click alla logica di battaglia
            HandleBattleClick(HitActor, true); // true = click sinistro
        break;

    default:
        // Se siamo in una fase non gestita, non fa nulla
            break;
    }
}

/**
 * Metodo: HandlePlacementClick
 * 
 * Metodo eseguito durante la fase di piazzamento quando il giocatore clicca su una tile.
 * Se è il turno del player e l’attore cliccato è una tile valida, si notifica al PlacementManager
 * di procedere al piazzamento dell’unità selezionata su quella tile.
 */
void AMyPlayerController::HandlePlacementClick(AActor* HitActor)
{
    // Se non è il turno del giocatore, ignora il click
    if (GameMode->TurnManager->GetCurrentPlayer() != EPlayer::Player1) return;

    // Ottiene il PlacementManager responsabile della logica di piazzamento
    APlacementManager* PlacementManager = GameMode->GetPlacementManager();
    if (!PlacementManager) return; // Se non esiste, esce

    // Verifica se l’attore cliccato è una tile (casella della griglia)
    if (ATile* ClickedTile = Cast<ATile>(HitActor))
    {
        // Se sì, passa la tile selezionata al PlacementManager per gestire il piazzamento dell’unità
        PlacementManager->HandleTileClick(ClickedTile);
    }
}

/**
* * 
 * Questo metodo gestisce il comportamento dei click (sinistro o destro) del giocatore durante la fase di battaglia.
 * È progettato per distinguere i seguenti casi:
 * 1. Click sinistro su tile valida → se un’unità player è selezionata e può muoversi, viene ordinato il movimento.
 * 2. Click sinistro su una tile non valida o fuori portata** → viene annullata la selezione dell’unità.
 * 3. Click sinistro o destro su un’unità del player**:
 *    - Se è la stessa già selezionata:
 *        - Click sinistro → deseleziona.
 *        - Click destro → mostra la griglia di attacco se può agire.
 *    - Se è una nuova unità:
 *        - Click sinistro → mostra griglia di movimento (solo se Idle).
 *        - Click destro → mostra griglia di attacco (solo se può agire).
 * 4. Click destro su un’unità nemica → se l’unità selezionata può agire e il nemico è in range, viene eseguito un attacco.
 * 
 * @param HitActor 
 * @param isLeft 
 */
void AMyPlayerController::HandleBattleClick(AActor* HitActor, bool isLeft)
{
    // Verifica che GameMode, TurnManager siano validi e che ci si trovi nella fase di battaglia
    if (!GameMode || !GameMode->TurnManager || GameMode->GetCurrentGamePhase() != EGamePhase::EBattle) return;

    // Se non è il turno del player o la griglia è bloccata, esce
    if (GameMode->TurnManager->GetCurrentPlayer() != EPlayer::Player1 || bIsGridLocked) return;

    // --- CASO 1: il giocatore clicca su una TILE (potenzialmente per muoversi) ---
    if (ATile* ClickedTile = Cast<ATile>(HitActor))
    {
        // Se è un click sinistro e un'unità è selezionata
        if (isLeft && SelectedUnit)
        {
            // Recupera le tile valide per il movimento dell’unità selezionata
            const TArray<ATile*> ValidTiles = GridManager->GetValidMovementTiles(SelectedUnit);

            // Se la tile cliccata è valida e l'unità non ha ancora agito
            if (ValidTiles.Contains(ClickedTile) && SelectedUnit->GetCurrentAction() == EUnitAction::Idle)
            {
                // Esegue il movimento
                TryMoveToTile(ClickedTile);
            }
            else
            {
                // Altrimenti annulla la selezione e pulisce le evidenziazioni
                GridManager->ClearHighlights();
                SelectedUnit = nullptr;
            }
        }
        return; // Fine gestione TILE
    }

    // --- CASO 2: Il giocatore clicca su un'UNITÀ ---
    else if (AUnitBase* ClickedUnit = Cast<AUnitBase>(HitActor))
    {
        // --- CASO 2A: Click su unità del PLAYER ---
        if (ClickedUnit->IsPlayerControlled())
        {
            // Click su unità già selezionata
            if (ClickedUnit == SelectedUnit)
            {
                if (isLeft)
                {
                    // Click sinistro sulla stessa unità → deselezione e pulizia
                    GridManager->ClearHighlights();
                    SelectedUnit = nullptr;
                }
                else if (SelectedUnit->CanAct())
                {
                    // Click destro sulla stessa unità → mostra griglia d’attacco
                    GridManager->ClearHighlights();
                    GridManager->HighlightTileUnderUnit(SelectedUnit, FLinearColor(0.8f, 0.4f, 0.0f)); // Tile arancione
                    GridManager->HighlightAttackGrid(SelectedUnit);
                }
            }
            else
            {
                // Click su unità diversa → cambio di selezione

                // Pulisce le griglie e aggiorna l’unità selezionata
                GridManager->ClearHighlights();
                SelectedUnit = ClickedUnit;

                // Evidenzia la tile sotto la nuova unità selezionata
                GridManager->HighlightTileUnderUnit(SelectedUnit, FLinearColor(0.8f, 0.4f, 0.0f));

                // Se può agire:
                if (SelectedUnit->CanAct())
                {
                    // Se è click sinistro e l'unità è in stato Idle → mostra griglia movimento
                    if (isLeft && SelectedUnit->GetCurrentAction() == EUnitAction::Idle)
                    {
                        GridManager->HighlightMovementTiles(SelectedUnit);
                    }
                    // Se è click destro → mostra griglia di attacco
                    else if (!isLeft)
                    {
                        GridManager->HighlightAttackGrid(SelectedUnit);
                    }
                }
            }
        }

        // --- CASO 2B: Click su unità NEMICA ---
        else
        {
            // Click destro su nemico, con un'unità selezionata che può attaccare
            if (!isLeft && SelectedUnit && SelectedUnit->CanAct())
            {
                // Recupera le tile che la nostra unità può attaccare
                TArray<ATile*> AttackTiles = GridManager->GetValidAttackTiles(SelectedUnit);
                ATile* TargetTile = GridManager->FindTileAtLocation(ClickedUnit->GetActorLocation());

                // Se il nemico è in una delle tile attaccabili
                if (AttackTiles.Contains(TargetTile))
                {
                    // Pulisce le evidenziazioni e avvia l'attacco
                    GridManager->ClearHighlights();
                    TryAttack(SelectedUnit, ClickedUnit);
                }
            }
        }
    }
}

/*
* Descrizione:
* Verifica se l'unità selezionata può attaccare un determinato nemico (Defender).
* Se il bersaglio si trova nella zona d'attacco dell'unità, allora esegue l'attacco.
* 
* Nota: viene chiamato durante la fase di battaglia con il click destro su una cella rossa evidenziata.
*/
void AMyPlayerController::TryAttack(AUnitBase* Attacker, AUnitBase* Defender)
{
    // Controlla che l'attaccante sia valido e che non abbia già attaccato o fatto move+attack
    if (!Attacker || Attacker->GetCurrentAction() == EUnitAction::Attacked || Attacker->GetCurrentAction() == EUnitAction::MoveAttack)
    {
        UE_LOG(LogTemp, Warning, TEXT(" %s non può attaccare: ha già attaccato o fatto move+attack."), *Attacker->GetName());
        return;
    }

    // Ottiene la tile su cui si trova il difensore
    ATile* TileDefender = GridManager->FindTileAtLocation(Defender->GetActorLocation());

    // Calcola l'area d'attacco valida per l'attaccante
    TArray<ATile*> AttackArea = GridManager->GetValidAttackTiles(Attacker);

    // Se il difensore si trova all'interno dell'area di attacco...
    if (AttackArea.Contains(TileDefender))
    {
        // ...esegui l'attacco
        ExecuteAttack(Attacker, Defender);
    }
}

/*
* Metodo: TryMoveToTile
* 
* Descrizione:
* Questo metodo tenta di muovere l'unità selezionata (SelectedUnit) verso una tile cliccata (ClickedTile),
* purché la tile sia raggiungibile e l'unità sia nello stato Idle. 
* Dopo il movimento, l'azione viene registrata nella history del turno.
*
* Nota: viene chiamato durante la fase di battaglia con il click sinistro su una cella azzurra evidenziata.
*/
void AMyPlayerController::TryMoveToTile(ATile* ClickedTile)
{
    // Controlla che ci sia un'unità selezionata e che possa ancora agire
    if (!SelectedUnit || !SelectedUnit->CanAct()) return;

    // Ottiene le tile su cui l'unità può muoversi
    const TArray<ATile*> ValidTiles = GridManager->GetValidMovementTiles(SelectedUnit);

    // Se la tile cliccata non è valida o l'unità ha già mosso in questo turno, esce
    if (!ValidTiles.Contains(ClickedTile) || SelectedUnit->bHasMovedThisTurn) return;

    // Calcola il percorso dalla posizione attuale alla tile cliccata
    const TArray<ATile*> Path = GridManager->GetPathToTile(SelectedUnit, ClickedTile);

    // Controlla che il percorso non sia più lungo del range di movimento dell'unità
    if (Path.Num() > SelectedUnit->GetMovementRange())
    {
        UE_LOG(LogTemp, Warning, TEXT("Percorso troppo lungo."));
        GridManager->ClearHighlights();  // Rimuove highlight se movimento non valido
        return;
    }

    // Rimuove eventuali evidenziazioni precedenti
    GridManager->ClearHighlights();

    // Imposta l'azione corrente come "Moved"
    SelectedUnit->SetCurrentAction(EUnitAction::Moved);

    // Blocca temporaneamente l’input del giocatore
    SetMovementLocked(true);

    // Ordina il movimento dell’unità lungo il percorso calcolato
    MovementManager->MoveUnit(SelectedUnit, Path, 300.f); // Velocità: 300.f

    // Recupera la tile di partenza
    ATile* From = GridManager->FindTileAtLocation(SelectedUnit->GetActorLocation());
    FString FromName = From ? From->GetTileIdentifier() : TEXT("???");

    // Identificativo della tile di destinazione
    FString ToName = ClickedTile->GetTileIdentifier();

    // Determina il tipo dell'unità per la scrittura nello storico
    FString UnitType = SelectedUnit->IsRangedAttack() ? TEXT("Sniper") : TEXT("Brawler");

    // Aggiunge la mossa alla history del turno
    GameMode->AddMoveToHistory(FString::Printf(TEXT("Player: %s moves from %s to %s"), *UnitType, *FromName, *ToName));
}

/*
* Descrizione:
* Gestisce il comportamento del click destro del mouse.
* A seconda della fase di gioco (piazzamento o battaglia), delega l'azione al metodo appropriato.
* Se il gioco è nella fase EGameOver, forza la visualizzazione del messaggio finale.
*/
void AMyPlayerController::OnRightClick()
{
    // Imposta il comportamento dell'input per non bloccare il click su UI
    FInputModeGameOnly InputMode;
    InputMode.SetConsumeCaptureMouseDown(false);
    SetInputMode(InputMode);

    // Se il gioco è finito, forza la chiamata alla gestione del GameOver
    if (GameMode->GetCurrentGamePhase() == EGamePhase::EGameOver)
    {
        GameMode->HandleGameOver(GameMode->PlayerUnits.Num() == 0 ? TEXT("AI") : TEXT("Player"));
    }

    // Se il movimento è bloccato o mancano riferimenti, interrompe
    if (bIsGridLocked || !GameMode || !GameMode->TurnManager) return;

    // Recupera GridManager se non già ottenuto
    if (!GridManager) GridManager = GameMode->GetGridManager();
    if (!GridManager) return;

    // Ottiene l’attore cliccato sotto il cursore del mouse
    FHitResult Hit;
    if (!GetHitResultUnderCursor(ECC_Visibility, false, Hit) || !Hit.GetActor()) return;
    AActor* HitActor = Hit.GetActor();

    // Switch in base alla fase del gioco
    switch (GameMode->GetCurrentGamePhase())
    {
    case EGamePhase::EPlacement:
        HandlePlacementClick(HitActor); // Click destro durante il piazzamento
        break;
    case EGamePhase::EBattle:
        HandleBattleClick(HitActor, false); // Click destro durante la battaglia
        break;
    default:
        break;
    }
}

/*
* Descrizione:
* Gestisce l'attacco vero e proprio tra un'unità Attacker e un'unità Defender.
* Aggiorna la UI, registra l'evento nello storico e gestisce eventuale contrattacco (solo per Sniper).
*/
void AMyPlayerController::ExecuteAttack(AUnitBase* Attacker, AUnitBase* Defender)
{
    // Se il gioco è finito, annulla l’attacco
    if (GameMode && GameMode->GetCurrentGamePhase() == EGamePhase::EGameOver)
    {
        UE_LOG(LogTemp, Warning, TEXT("Turno IA interrotto: gioco terminato."));
        return;
    }

    // Controllo di validità
    if (!Attacker || !Defender) return;

    // Recupera il widget dello stato (UI)
    UStatusGameWidget* StatusGame = nullptr;
    if (GameMode)
    {
        StatusGame = GameMode->GetStatusGameWidget();
    }

    // Blocca temporaneamente l’input del giocatore
    SetMovementLocked(true);

    // Esegue l’attacco tra le due unità
    Attacker->AttackUnit(Defender);

    // Crea il messaggio da mostrare nello storico
    ATile* DefenderTile = GridManager->FindTileAtLocation(Defender->GetActorLocation());
    FString TileName = DefenderTile ? DefenderTile->GetTileIdentifier() : TEXT("???");
    FString UnitType = Attacker->IsRangedAttack() ? TEXT("Sniper") : TEXT("Brawler");
    int32 Damage = FMath::RandRange(Attacker->MinDamage, Attacker->MaxDamage); // Simula il danno per la UI

    GameMode->AddMoveToHistory(FString::Printf(TEXT("Player: %s attacks %s damage %d"), *UnitType, *TileName, Damage));

    // Imposta l’azione corrente come "Attacked"
    Attacker->SetCurrentAction(EUnitAction::Attacked);

    // Log dell'attacco
    UE_LOG(LogTemp, Warning, TEXT(" %s ha attaccato %s"), *Attacker->GetName(), *Defender->GetName());

    // Controlla se va eseguito il contrattacco (solo se l'attaccante è uno Sniper)
    if (Attacker->IsRangedAttack())
    {
        const bool DefenderIsSniper = Defender->IsRangedAttack();
        const bool DefenderIsBrawlerClose =
            !Defender->IsRangedAttack() &&
            FVector::Dist2D(Attacker->GetActorLocation(), Defender->GetActorLocation()) <= 110.f;

        // Se le condizioni sono rispettate, esegue il contrattacco
        if (DefenderIsSniper || DefenderIsBrawlerClose)
        {
            int32 CounterDamage = FMath::RandRange(1, 3); // Danno contrattacco casuale
            UE_LOG(LogTemp, Warning, TEXT(" Contrattacco! %s infligge %d danni a %s"), *Defender->GetName(), CounterDamage, *Attacker->GetName());

            // Applica il danno all'attaccante
            FPointDamageEvent DamageEvent;
            Attacker->TakeDamage(CounterDamage, DamageEvent, nullptr, Defender);

            // Mostra contrattacco nella history
            ATile* AttackerTile = GridManager->FindTileAtLocation(Attacker->GetActorLocation());
            TileName = AttackerTile ? AttackerTile->GetTileIdentifier() : TEXT("???");
            FString DefenderType = Defender->IsRangedAttack() ? TEXT("Sniper") : TEXT("Brawler");

            GameMode->AddMoveToHistory(FString::Printf(TEXT("AI: %s attacks %s damage %d"), *DefenderType, *TileName, CounterDamage));
        }

        // Controlla se l'attaccante è morto dopo il contrattacco
        if (Attacker->IsDead())
        {
            Attacker->Die(Attacker);
        }
        else if (StatusGame)
        {
            // Aggiorna la barra della vita
            StatusGame->UpdateUnitHealth(Attacker, Attacker->GetHealthPercent());
        }
    }

    // Rimuove gli highlight e deseleziona l'unità
    GridManager->ClearHighlights();
    SelectedUnit = nullptr;

    // Notifica il TurnManager che è stato eseguito un attacco
    if (GameMode && GameMode->TurnManager)
    {
        GameMode->TurnManager->RegisterPlayerAttack(Attacker);
    }

    // Sblocca l’input
    SetMovementLocked(false);
}

/*
* Descrizione:
* Chiamato automaticamente quando un'unità termina il movimento.
* Deseleziona l’unità, registra la mossa nel TurnManager e sblocca l’input.
*/
void AMyPlayerController::OnUnitMovementFinished(AUnitBase* Unit)
{
    // Deseleziona l'unità dopo il movimento
    SelectedUnit = nullptr;

    // Notifica al TurnManager che l’unità ha completato il movimento
    if (GameMode && GameMode->TurnManager)
    {
        GameMode->TurnManager->RegisterPlayerMove(Unit);
    }

    // Sblocca l’input per permettere nuove azioni
    SetMovementLocked(false);
}

/*
* Descrizione:
* Attiva o disattiva il blocco dell’input del giocatore.
* Questo viene usato per evitare click multipli durante animazioni di movimento o attacco.
*/
void AMyPlayerController::SetMovementLocked(bool bLocked)
{
    bIsGridLocked = bLocked;

    // Log per confermare lo stato attuale dell’input
    UE_LOG(LogTemp, Warning, TEXT("%s input player"), bLocked ? TEXT("Blocco") : TEXT("Sblocco"));
}