// Creato da: Schifano Francesco, 5469994

#include "PlacementManager.h"
#include "MyGameMode.h"
#include "TurnManager.h"
#include "PAASchifanoFrancesco/Units/Brawler.h"
#include "PAASchifanoFrancesco/Units/Sniper.h"
#include "PAASchifanoFrancesco/Grid/GridManager.h"
#include "PAASchifanoFrancesco/Grid/Tile.h"
#include "PAASchifanoFrancesco/UI/SelectPawn.h"
#include "Kismet/GameplayStatics.h"

/**
 * Costruttore di APlacementManager.
 * Disattiva il tick (non serve aggiornamento ogni frame)
 * e inizializza il GridManager a nullptr.
 */
APlacementManager::APlacementManager()
{
    PrimaryActorTick.bCanEverTick = false; // Non serve il tick per questo attore
    GridManager = nullptr;                 // GridManager verrà assegnato successivamente
}

/**
 * Inizializza la classe salvando la SelectPawn UI e il riferimento al GameMode.
 * 
 * @param InSelectPawnClass - La classe widget da istanziare per la selezione delle unità.
 */
void APlacementManager::Initialize(TSubclassOf<USelectPawn> InSelectPawnClass)
{
    SelectPawnClass = InSelectPawnClass; // Memorizza la classe widget
    GM = Cast<AMyGameMode>(UGameplayStatics::GetGameMode(this)); // Recupera il GameMode corrente e lo casta
}

/**
 * Avvia la fase di piazzamento:
 * - Ottiene il riferimento al GridManager
 * - Crea e visualizza il widget per la selezione delle unità
 * - Avvia il turno tramite TurnManager
 */
void APlacementManager::StartPlacement()
{
    GridManager = GM->GetGridManager(); // Ottiene il riferimento alla griglia

    if (SelectPawnClass) // Verifica che la classe del widget sia valida
    {
        SelectPawn = CreateWidget<USelectPawn>(GetWorld(), SelectPawnClass); // Crea il widget per selezione unità

        if (SelectPawn)
        {
            SelectPawn->AddToViewport(); // Aggiunge il widget alla UI
        }
    }

    GM->TurnManager->StartTurn(); // Avvia il turno iniziale
}

/**
 * Gestisce il click su una tile durante la fase di piazzamento.
 * 
 * @param ClickedTile - La tile cliccata dal giocatore
 */
void APlacementManager::HandleTileClick(ATile* ClickedTile)
{
    // Verifica che sia presente una tile cliccata e una classe selezionata per il pawn
    if (!ClickedTile || !PlayerPawnType)
    {
        UE_LOG(LogTemp, Error, TEXT("HandleTileClick: Missing tile or pawn type"));
        return;
    }

    if (!GM || !GM->TurnManager) return; // Controllo sicurezza

    // Verifica che il gioco sia effettivamente nella fase di piazzamento
    if (GM->GetCurrentGamePhase() != EGamePhase::EPlacement)
    {
        UE_LOG(LogTemp, Warning, TEXT("Click ignorato: non siamo nella fase di piazzamento."));
        return;
    }

    // Se è il turno del Player1, si procede con il piazzamento
    if (GM->TurnManager->GetCurrentPlayer() == EPlayer::Player1)
    {
        UE_LOG(LogTemp, Warning, TEXT("Tentativo di piazzare un'unità di tipo: %s"), *PlayerPawnType->GetName());

        PlacePlayerPawn(ClickedTile); // Piazzamento dell'unità del player
    }
}

/**
 * Conclude la fase di piazzamento:
 * - Rimuove il widget di selezione unità
 * - Passa alla fase successiva (battaglia)
 */
void APlacementManager::FinishPlacementPhase()
{
    if (SelectPawn)
    {
        SelectPawn->RemoveFromParent(); // Rimuove il widget dalla UI
        SelectPawn = nullptr;
    }

    if (GM)
    {
        GM->SetGamePhase(EGamePhase::EBattle); // Passaggio alla fase di battaglia
    }
}

/**
 * Esegue il piazzamento di un'unità del giocatore su una tile valida.
 *
 * @param ClickedTile - La tile selezionata per il piazzamento
 */
void APlacementManager::PlacePlayerPawn(ATile* ClickedTile)
{
    // Verifica che il tipo di unità sia selezionato e che la tile sia valida
    if (!PlayerPawnType || ClickedTile->IsObstacle() || ClickedTile->GetHasPawn())
    {
        UE_LOG(LogTemp, Error, TEXT("PlacePlayerPawn: SelectedPawnType is null."));
        return;
    }

    // Ulteriore verifica per sicurezza
    if (ClickedTile->IsObstacle() || ClickedTile->GetHasPawn())
    {
        UE_LOG(LogTemp, Error, TEXT("Cella già occupata"));
        return;
    }

    // Calcola posizione di spawn
    FVector SpawnLocation = ClickedTile->GetPawnSpawnLocation();
    UE_LOG(LogTemp, Warning, TEXT("Posizione di spawn calcolata: %s"), *SpawnLocation.ToString());

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    SpawnParams.bNoFail = true;

    // Spawn dell’unità
    AUnitBase* NewPawn = GetWorld()->SpawnActor<AUnitBase>(PlayerPawnType, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
    
    if (!NewPawn)
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnActor ha fallito per: %s"), *PlayerPawnType->GetName());
    }

    // Imposta il nome a seconda del tipo di unità
    if (NewPawn->IsA(ASniper::StaticClass()))
    {
        NewPawn->UnitDisplayName = TEXT("Sniper(Player)");
    }
    else if (NewPawn->IsA(ABrawler::StaticClass()))
    {
        NewPawn->UnitDisplayName = TEXT("Brawler(Player)");
    }

    // Aggiorna stato della tile e dell'unità
    ClickedTile->SetHasPawn(true);
    NewPawn->SetIsPlayerController(true);

    // Registra la mossa nel TurnManager
    GM->TurnManager->RegisterPlacementMove(NewPawn);

    // Disabilita il bottone corrispondente alla pedina usata
    if (SelectPawn)
    {
        SelectPawn->DisableButtonForPawn(PlayerPawnType);
    }

    UE_LOG(LogTemp, Warning, TEXT("Player Pawn Placed: %s at %s"), *NewPawn->GetName(), *SpawnLocation.ToString());

    // Resetta la selezione corrente
    PlayerPawnType = nullptr;
}

/**
 * Piazza un'unità IA in modo automatico su una tile libera e non occupata.
 */
void APlacementManager::PlaceAIPawn()
{
    // Controlla se il GridManager è valido (non nullptr)
    if (!GridManager)
    {
        // Log di errore nel caso in cui GridManager non sia stato inizializzato
        UE_LOG(LogTemp, Error, TEXT("PlaceAIPawn: GridManager non valido!"));
        return; // Termina la funzione perché non è possibile proseguire senza la griglia
    }

    // Array che conterrà tutte le tile disponibili per il piazzamento
    TArray<ATile*> AvailableTiles;

    // Itera su tutte le tile della griglia
    for (ATile* Tile : GridManager->GetGridTiles())
    {
        // Condizione: la tile deve essere libera (senza ostacoli e senza altre unità)
        if (!Tile->IsObstacle() && !Tile->GetHasPawn())
        {
            // Aggiunge la tile all'elenco di quelle disponibili
            AvailableTiles.Add(Tile);
        }
    }

    // Se non ci sono tile disponibili, stampa un messaggio e interrompe la funzione
    if (AvailableTiles.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlaceAIPawn: Nessuna Tile disponibile per posizionare l'IA."));
        return;
    }

    // Sceglie una tile casuale dall'elenco di quelle disponibili
    int32 RandomIndex = FMath::RandRange(0, AvailableTiles.Num() - 1);
    ATile* ChosenTile = AvailableTiles[RandomIndex];

    // Controlla che la tile selezionata non sia nulla (per sicurezza)
    if (!ChosenTile)
    {
        UE_LOG(LogTemp, Error, TEXT("PlaceAIPawn: La Tile selezionata è nulla!"));
        return;
    }

    // Ottiene la posizione di spawn dalla tile selezionata
    FVector SpawnLocation = ChosenTile->GetPawnSpawnLocation();

    // Parametri di spawn per l'attore (unità IA)
    FActorSpawnParameters SpawnParams;
    // Forza lo spawn anche in caso di collisioni
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // Decide il tipo di unità IA da spawnare:
    // Se nessuna IA è ancora stata spawnata → Sniper, altrimenti → Brawler
    AIPawnType = (GM->AIUnits.Num() == 0) ? ASniper::StaticClass() : ABrawler::StaticClass();

    // Spawna effettivamente l'unità IA nel mondo
    AUnitBase* AIPawn = GetWorld()->SpawnActor<AUnitBase>(
        AIPawnType,         // Tipo dell'unità da spawnare (Sniper o Brawler)
        SpawnLocation,      // Posizione di spawn
        FRotator::ZeroRotator, // Rotazione iniziale (zero)
        SpawnParams         // Parametri di spawn definiti sopra
    );

    // Se lo spawn ha avuto successo...
    if (AIPawn)
    {
        // Imposta il nome visualizzato a schermo per distinguere il tipo
        if (AIPawn->IsA(ASniper::StaticClass()))
        {
            AIPawn->UnitDisplayName = TEXT("Sniper (AI)");
        }
        else if (AIPawn->IsA(ABrawler::StaticClass()))
        {
            AIPawn->UnitDisplayName = TEXT("Brawler (AI)");
        }

        // Aggiorna la tile selezionata per indicare che ora contiene un'unità
        ChosenTile->SetHasPawn(true);

        // Registra la nuova unità nel TurnManager (per tracciamento turno IA)
        GM->TurnManager->RegisterPlacementMove(AIPawn);

        // Log di conferma: stampa il nome e la posizione della nuova unità IA
        UE_LOG(LogTemp, Warning, TEXT("AI Pawn Placed: %s at %s"), *AIPawn->GetName(), *SpawnLocation.ToString());
    }
    else // Se lo spawn ha fallito...
    {
        // Log di errore
        UE_LOG(LogTemp, Error, TEXT("PlaceAIPawn: Fallito il posizionamento del Pawn IA!"));
    }
}

/**
 * Imposta il tipo di pedina da utilizzare nel piazzamento, sia per Player che per IA.
 *
 * @param PawnType - Classe della pedina da piazzare
 * @param Player - Player1 o AI
 */
void APlacementManager::SetSelectedPawnType(TSubclassOf<AUnitBase> PawnType, EPlayer Player)
{
    if (Player == EPlayer::Player1)
    {
        PlayerPawnType = PawnType; // Salva la classe per il Player
    }
    else if (Player == EPlayer::AI)
    {
        AIPawnType = PawnType;     // Salva la classe per l'IA
    }

    UE_LOG(LogTemp, Warning, TEXT("SelectedPawnType set to: %s for player %s"), *PawnType->GetName(), *UEnum::GetValueAsString(Player));
}
