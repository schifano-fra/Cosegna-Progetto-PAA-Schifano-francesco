// Creato da: Schifano Francesco 5469994
/*
* Classe: ABattleManager
* 
* Descrizione generale:
* Questa classe gestisce la logica di combattimento del gioco, coordinando i turni dell'AI,
* il movimento delle unità e gli attacchi. Funziona da intermediario tra il GameMode,
* il TurnManager, il GridManager e le singole unità.
* 
* Principali funzionalità:
* - Gestione del flusso di battaglia (inizio turni, fine turni)
* - Implementazione dell'AI per il movimento e l'attacco delle unità nemiche
* - Coordinamento tra gli altri manager del gioco
* - Visualizzazione delle aree di movimento/attacco
* - Registrazione delle azioni per l'history del gioco
*
* 
* Differenza tra IA Easy e IA Hard
* --------------------------------
* Il comportamento dell'intelligenza artificiale varia in base al livello selezionato:
*
* ► IA EASY:
*   - Movimento: sceglie una cella casuale tra quelle raggiungibili.
*   - Attacco: ha una probabilità casuale (50%) di attaccare dopo essersi mossa.
*   - Non ottimizza il posizionamento né valuta la priorità dei bersagli.
*   - È pensata per comportamenti semplici, prevedibili e non troppo intelligenti.
*   - L'AI di questa fifficoltà sorteggia All'iniio di ProcessNextAIUnit() se eseguire solo un moviemnto
*     oppure se eseguire un movimento e provare ad attacacre. Questo sarà molto negativo se AI all'inzio poteva
*     attaccare ma non lo fa sicuramente e poi movendosi non può più attccare. Il che la rende più facile
*     da sconfiggere.
*
* ► IA HARD (stesso comportamento di NORMAL):
*   - Attacco: prova sempre prima ad attaccare direttamente se possibile.
*   - Movimento: se non può attaccare subito, si sposta verso il nemico più vicino
*               usando un pathfinding ottimizzato.
*   - Dopo il movimento, prova di nuovo ad attaccare.
*   - Questo comportamento simula un'IA più strategica e aggressiva, che usa il proprio turno in modo efficiente.
*
* Questi comportamenti sono implementati nel metodo ProcessNextAIUnit() e variano in base al valore di GameMode->AILevel.
*/

#include "BattleManager.h"
#include "MyGameMode.h"
#include "TurnManager.h"
#include "PAASchifanoFrancesco/Units/UnitBase.h"
#include "PAASchifanoFrancesco/Grid/GridManager.h"
#include "PAASchifanoFrancesco/Grid/Tile.h"
#include "PAASchifanoFrancesco/Units/UnitMovementManager.h"
#include "PAASchifanoFrancesco/UI/StatusGameWidget.h"
#include "Kismet/GameplayStatics.h"

/*
* Costruttore della classe ABattleManager
* Inizializza le variabili membro a nullptr e disabilita il tick
*/
ABattleManager::ABattleManager()
{
    PrimaryActorTick.bCanEverTick = false;
    SelectedUnit = nullptr; // Inizializza l’unità selezionata a nullptr
    TurnManager = nullptr; // Inizializza il TurnManager a nullptr
}

/*
* Metodo: StartBattle
* 
* Descrizione:
* Avvia la battaglia recuperando i riferimenti ai vari manager necessari
* e impostando lo stato iniziale del gioco.
* Flusso:
* 1. Ottiene il GameMode e i manager collegati
* 2. Verifica la presenza del MovementManager
* 3. Avvia il primo turno
* 4. Aggiorna lo stato delle unità nell'UI
*/
void ABattleManager::StartBattle()
{
    GameMode = Cast<AMyGameMode>(UGameplayStatics::GetGameMode(this)); // Recupera GameMode attivo
    
    if (GameMode)
    {
        GridManager = GameMode->GetGridManager();  // Ottiene il GridManager
        TurnManager = GameMode->GetTurnManager(); // Ottiene TurnManager
        MovementManager = GameMode->GlobalMovementManager;  // Ottiene MovementManager globale
    }
    
    if (!MovementManager)
    {
        MovementManager = GameMode->GlobalMovementManager;
        if (!MovementManager)
        {
            UE_LOG(LogTemp, Error, TEXT("MovementManager NON TROVATO in BattleManager!")); // Fermiamo tutto se non c'è
            return;
        }
        MovementManager->OnMovementFinished.AddDynamic(this, &ABattleManager::OnAIMovementComplete); // Associa delegato che notifica la fine di un movimento AI
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Battaglia iniziata. Primo turno: %s"), *UEnum::GetValueAsString(GameMode->TurnManager->GetCurrentPlayer()));

    GameMode->TurnManager->StartTurn(); // Avvia il turno nel TurnManager

    if (GameMode && GameMode->GetStatusGameWidget()) // Se il widget dello stato è disponibile
    {
        for (AUnitBase* Unit : GameMode->PlayerUnits) // Aggiunge le unità del giocatore al widget
        {
            GameMode->GetStatusGameWidget()->AddUnitStatus(Unit);
        }
        for (AUnitBase* Unit : GameMode->AIUnits) // Aggiunge le unità AI al widget
        {
            GameMode->GetStatusGameWidget()->AddUnitStatus(Unit);
        }
    }
}

/*
* Metodo: OnAIMovementComplete
* 
* Descrizione:
* Callback chiamata quando un'unità AI completa il movimento.
* Pulisce le highlight dalla griglia.
*/
void ABattleManager::OnAIMovementComplete(AUnitBase* UnitBase)
{
    GridManager->ClearHighlights(); // Pulisce gli highlight della griglia
}

/*
* Descrizione:
* Prepara il turno dell'AI inizializzando la lista delle unità da processare
* e avviando il processing della prima unità.
*/
void ABattleManager::PrepareAITurn()
{
    if (!GameMode || !TurnManager || !GridManager) return; // Controllo di sicurezza

    UE_LOG(LogTemp, Warning, TEXT("AI inizia il suo turno..."));

    AIUnitsToProcess = GameMode->AIUnits; // Copia le unità AI da processare
    CurrentAIIndex = 0; // Inizia dal primo indice

    ProcessNextAIUnit(); // Avvia la gestione dell'unità
}

/*
* Descrizione:
* Gestisce il comportamento di una singola unità AI durante il suo turno.
* 
* Flusso principale:
* 1. Mostra le tile di movimento disponibili
* 2. Dopo un delay, mostra le tile di attacco
* 3. In base al livello di difficoltà, sceglie se:
*    - Muoversi casualmente (Easy)
*    - Muoversi verso il nemico più vicino e attaccare (Hard)
* 4. Passa alla prossima unità o termina il turno AI
*/
void ABattleManager::ProcessNextAIUnit()
{
    // Se tutte le unità AI sono già state processate, termina il turno
    if (CurrentAIIndex >= AIUnitsToProcess.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("Tutte le unità AI hanno agito. Fine turno AI."));
        TurnManager->EndTurn(); // Chiusura turno AI
        return;
    }

    // Recupera l'unità AI corrente
    AUnitBase* CurrentUnit = AIUnitsToProcess[CurrentAIIndex];

    // Se l'unità è nulla, non può agire o ha già attaccato, passa alla successiva
    if (!CurrentUnit || !CurrentUnit->CanAct() || CurrentUnit->GetCurrentAction() == EUnitAction::Attacked)
    {
        CurrentAIIndex++;
        ProcessNextAIUnit(); // Passa alla prossima unità
        return;
    }

    // Mostra la griglia di movimento per l’unità corrente
    GridManager->HighlightMovementTiles(CurrentUnit);

    // Timer: mostra la griglia di movimento per 3 secondi
    FTimerHandle MovementTimerHandle;
    GameMode->GetWorld()->GetTimerManager().SetTimer(MovementTimerHandle, [this, CurrentUnit]()
    {
        GridManager->ClearHighlights(); // Rimuove highlight movimento

        // Mostra la griglia di attacco
        GridManager->HighlightAttackGrid(CurrentUnit);

        // Timer: mostra la griglia d'attacco per 3 secondi
        FTimerHandle AttackTimerHandle;
        GameMode->GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, [this, CurrentUnit]()
        {
            GridManager->ClearHighlights(); // Rimuove highlight attacco

            // ---- AI LEVEL: EASY ----
            if (GameMode && GameMode->AILevel == EAILevel::Easy)
            {
                TArray<ATile*> ReachableTiles = GridManager->GetValidMovementTiles(CurrentUnit);

                if (ReachableTiles.Num() <= 1) // Nessuna tile utile oltre quella in cui si trova
                {
                    UE_LOG(LogTemp, Warning, TEXT("AI Easy: nessuna tile disponibile per muoversi"));

                    if (!TryAIAttack(CurrentUnit))
                    {
                        // Non può nemmeno attaccare → salta
                        FTimerHandle SkipHandle;
                        GameMode->GetWorld()->GetTimerManager().SetTimer(SkipHandle, [this]()
                        {
                            CurrentAIIndex++;
                            ProcessNextAIUnit();
                        }, 1.0f, false);
                    }
                    else
                    {
                        // Ha attaccato, va comunque avanti
                        CurrentAIIndex++;
                        ProcessNextAIUnit();
                    }
                    return;
                }

                // Movimento possibile → decidi se muovere o muovere+attaccare
                int32 Choice = FMath::RandRange(0, 1); // 0 = solo muovi, 1 = muovi+attacca

                TryAIRandomMove(CurrentUnit); // Effettua movimento random

                if (Choice == 1)
                {
                    // Attende 3s e poi prova ad attaccare
                    FTimerHandle EasyAttackTimer;
                    GameMode->GetWorld()->GetTimerManager().SetTimer(EasyAttackTimer, [this, CurrentUnit]()
                    {
                        GridManager->HighlightAttackGrid(CurrentUnit);
                        TryAIAttack(CurrentUnit);

                        FTimerHandle AfterAttack;
                        GameMode->GetWorld()->GetTimerManager().SetTimer(AfterAttack, [this]()
                        {
                            GridManager->ClearHighlights();
                            CurrentAIIndex++;
                            ProcessNextAIUnit();
                        }, 3.0f, false);
                    }, 3.0f, false);
                    return;
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("AI Easy: solo movimento"));

                    FTimerHandle EasyEndTimer;
                    GameMode->GetWorld()->GetTimerManager().SetTimer(EasyEndTimer, [this]()
                    {
                        CurrentAIIndex++;
                        ProcessNextAIUnit();
                    }, 3.0f, false);
                    return;
                }
            }
            else if (GameMode && GameMode->AILevel == EAILevel::Hard)
            {
                // ---- AI LEVEL: NORMAL/HARD ----

                // Prova a fare attacco diretto
                if (!TryAIAttack(CurrentUnit))
                {
                    // Se non ha attaccato → prova a muoversi
                    TryAIMove(CurrentUnit);

                    // Dopo 5s → riprova ad attaccare
                    FTimerHandle AfterMoveHandle;
                    GameMode->GetWorld()->GetTimerManager().SetTimer(AfterMoveHandle, [this, CurrentUnit]()
                    {
                        GridManager->HighlightAttackGrid(CurrentUnit);
                        TryAIAttack(CurrentUnit);

                        FTimerHandle AttackTimer;
                        GameMode->GetWorld()->GetTimerManager().SetTimer(AttackTimer, [this]()
                        {
                            GridManager->ClearHighlights();
                            CurrentAIIndex++;
                            ProcessNextAIUnit();
                        }, 1.0f, false);
                    }, 5.0f, false);
                }
                else
                {
                    // Ha attaccato subito, passa avanti
                    CurrentAIIndex++;
                    ProcessNextAIUnit();
                }   
            }
        }, 3.0f, false); // Delay attacco

    }, 3.0f, false); // Delay movimento
}

/*
* Metodo: TryAIMove
* 
* Descrizione:
* Tenta di muovere un'unità AI verso il nemico più vicino.
* Calcola il percorso ottimale considerando il range di movimento.
* 
* Flusso:
* 1. Trova il nemico più vicino
* 2. Calcola il percorso verso di esso
* 3. Determina le tile raggiungibili
* 4. Esegue il movimento lungo il percorso
* 5. Registra l'azione nella history
*/
void ABattleManager::TryAIMove(AUnitBase* AIUnit)
{
    if (!AIUnit || !GridManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("TryAIMove: AIUnit o GridManager è nullo"));
        return;
    }

    AUnitBase* Enemy = FindNearestEnemy(AIUnit);  // Trova il nemico più vicino
    if (!Enemy)
    {
        UE_LOG(LogTemp, Warning, TEXT("TryAIMove: Nessun nemico trovato"));
        return;
    }

    ATile* EnemyTile = GridManager->FindTileAtLocation(Enemy->GetActorLocation()); // Ottiene la tile del nemico
    if (!EnemyTile)
    {
        UE_LOG(LogTemp, Warning, TEXT("TryAIMove: EnemyTile non trovato"));
        return;
    }

    TArray<ATile*> Path = GridManager->GetPathToTile(AIUnit, EnemyTile);  // Calcola il percorso
    TArray<ATile*> Area = GridManager->GetValidMovementTiles(AIUnit); // Ottiene le tile raggiungibili

    int32 MaxSteps = AIUnit->GetMovementRange(); // Ottiene il range massimo
    int32 LastReachableIndex = -1; // Indice della destinazione valida

    for (int32 i = FMath::Min(MaxSteps - 1, Path.Num() - 1); i >= 0; i--)
    {
        if (Area.Contains(Path[i]))
        {
            LastReachableIndex = i; // Ultima tile valida
            break;
        }
    }
    
    if (LastReachableIndex != -1)
    {
        TArray<ATile*> PathToMove;
        for (int32 i = 0; i <= LastReachableIndex; i++)
        {
            PathToMove.Add(Path[i]); // Costruisce il path effettivo da percorrere
        }

        MovementManager->MoveUnit(AIUnit, PathToMove, 300.f);

        ATile* From = GridManager->FindTileAtLocation(AIUnit->GetActorLocation()); // Tile di partenza
        FString FromName = From ? From->GetTileIdentifier() : TEXT("???");
        FString ToName = PathToMove.Last()->GetTileIdentifier();  // Tile di arrivo
        FString UnitType = AIUnit->IsRangedAttack() ? TEXT("Sniper") : TEXT("Brawler");  // Tipo unità
        GameMode->AddMoveToHistory(FString::Printf(TEXT("AI: %s moves from %s to %s"), *UnitType, *FromName, *ToName)); // Registra l'azione

        TurnManager->RegisterAIMove(AIUnit); // Notifica che si è mossa
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Nessuna tile raggiungibile trovata nel path."));
    }
}


/*
* Metodo: TryAIRandomMove
* 
* Descrizione:
* Muove un'unità AI in una posizione casuale all'interno del suo range.
* Utilizzato principalmente per il livello di difficoltà Easy.
*/
void ABattleManager::TryAIRandomMove(AUnitBase* AIUnit)
{
    if (!AIUnit || !GridManager) return; // Verifica validità riferimenti

    TArray<ATile*> MovableTiles = GridManager->GetValidMovementTiles(AIUnit); // Ottiene le tile valide
    if (MovableTiles.Num() == 0) return; // Nessuna tile disponibile

    int32 Index = FMath::RandRange(0, MovableTiles.Num() - 1); // Sceglie un indice casuale
    ATile* Destination = MovableTiles[Index]; // Tile di destinazione

    TArray<ATile*> Path = GridManager->GetPathToTile(AIUnit, Destination); // Percorso da seguire
    MovementManager->MoveUnit(AIUnit, Path, 300.f); // Esegue il movimento

    FString From = GridManager->FindTileAtLocation(AIUnit->GetActorLocation()) ?
        GridManager->FindTileAtLocation(AIUnit->GetActorLocation())->GetTileIdentifier() : TEXT("???");

    FString To = Destination ? Destination->GetTileIdentifier() : TEXT("???");
    FString UnitType = AIUnit->IsRangedAttack() ? TEXT("Sniper") : TEXT("Brawler");

    GameMode->AddMoveToHistory(FString::Printf(TEXT("AI (Easy): %s moves from %s to %s"), *UnitType, *From, *To));
    TurnManager->RegisterAIMove(AIUnit); // Notifica movimento effettuato
}

/*
* Metodo: TryAIAttack
* Creato da: Schifano Francesco 5469994
* 
* Descrizione:
* Tenta di eseguire un attacco con l'unità AI se ci sono nemici nel range.
* Restituisce true se è stato eseguito un attacco, false altrimenti.
*/
bool ABattleManager::TryAIAttack(AUnitBase* AIUnit)
{
    TArray<ATile*> AttackTiles = GridManager->GetValidAttackTiles(AIUnit); // Ottiene le tile d'attacco

    for (AUnitBase* PlayerUnit : GameMode->PlayerUnits) // Cicla sulle unità nemiche
    {
        ATile* PlayerTile = GridManager->FindTileAtLocation(PlayerUnit->GetActorLocation());

        if (!PlayerTile) continue; // Salta se tile non trovata

        if (AttackTiles.Contains(PlayerTile)) // Se il nemico è attaccabile
        {
            UE_LOG(LogTemp, Warning, TEXT("AI %s attacca %s"), *AIUnit->GetName(), *PlayerUnit->GetName());
            AIUnit->AttackUnit(PlayerUnit); // Esegue l'attacco
            // Aggiorna la barra della vita del difensore
            if (GameMode && GameMode->GetStatusGameWidget() && !PlayerUnit->IsDead())
            {
                GameMode->GetStatusGameWidget()->UpdateUnitHealth(PlayerUnit, PlayerUnit->GetHealthPercent());
            }

            ATile* TargetTile = GridManager->FindTileAtLocation(PlayerUnit->GetActorLocation());
            FString TileName = TargetTile ? TargetTile->GetTileIdentifier() : TEXT("???");
            FString UnitType = AIUnit->IsRangedAttack() ? TEXT("Sniper") : TEXT("Brawler");
            int32 Damage = FMath::RandRange(AIUnit->MinDamage, AIUnit->MaxDamage); // Calcola danno
            GameMode->AddMoveToHistory(FString::Printf(TEXT("AI: %s attacks %s damage %d"), *UnitType, *TileName, Damage));

            TurnManager->RegisterAIAttack(AIUnit); // Notifica attacco
            return true;
        }
    }
    return false; // Nessun attacco eseguito
}

/*
* Metodo: FindNearestEnemy
* 
* Descrizione:
* Trova l'unità nemica più vicina all'unità AI specificata.
* Restituisce nullptr se non trova nemici.
*/
AUnitBase* ABattleManager::FindNearestEnemy(AUnitBase* AIUnit)
{
    if (!AIUnit)
    {
        UE_LOG(LogTemp, Error, TEXT("FindNearestEnemy: Unità AI non valida!"));
        return nullptr;
    }

    AUnitBase* NearestEnemy = nullptr;
    float MinDistance = FLT_MAX; // Distanza iniziale massima

    for (AUnitBase* PlayerUnit : GameMode->PlayerUnits)
    {
        if (PlayerUnit && PlayerUnit->IsPlayerControlled()) 
        {
            float Distance = FVector::Dist2D(AIUnit->GetActorLocation(), PlayerUnit->GetActorLocation()); // Distanza 2D
            if (Distance < MinDistance)
            {
                MinDistance = Distance;
                NearestEnemy = PlayerUnit; // Aggiorna il più vicino
            }
        }
    }
    return NearestEnemy;  // Ritorna l'unità trovata
}