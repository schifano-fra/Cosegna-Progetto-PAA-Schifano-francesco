#include "GridManager.h"
#include "PAASchifanoFrancesco/Units/UnitBase.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraActor.h"
#include "Tile.h"
#include "PAASchifanoFrancesco/Core/TurnManager.h"
#include "PAASchifanoFrancesco/Core/MyGameMode.h"
#include "Camera/CameraComponent.h"
#include "Components/LightComponent.h"
#include "Engine/DirectionalLight.h"

/** 
 * Costruttore del GridManager
 * 
 * In questo costruttore si inizializzano tutte le variabili fondamentali per il gestore della griglia:
 * - si disattiva il Tick (non serve aggiornare ad ogni frame),
 * - si inizializzano i puntatori a nullptr,
 * - si genera una percentuale casuale per il numero di ostacoli (per rendere la generazione della griglia non prevedibile),
 * - si crea un componente root vuoto per ancorare le tile della griglia.
 */
AGridManager::AGridManager()
{
    PrimaryActorTick.bCanEverTick = false; // Disabilitiamo il tick perché la griglia non richiede aggiornamenti per frame
    
    TurnManager = nullptr;         // Il TurnManager verrà recuperato nel BeginPlay
    bAttackGridVisible = false;    // Nessuna griglia d’attacco è visibile all’avvio

    // Percentuale casuale di ostacoli per ogni nuova partita (fra 30% e 95%)
    float RandomPercent = FMath::FRandRange(0.3f, 0.95f);
    ObstaclePercentage = FMath::Clamp(RandomPercent, 0.3f, 0.95f); // Clamp per sicurezza

    // Aggiungiamo un componente "root" che fungerà da genitore per tutte le tile
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

/** 
 * Metodo chiamato automaticamente quando l'attore viene posizionato nel mondo o quando il gioco inizia.
 * 
 * Questo metodo:
 * - Inizializza il GameMode e recupera il TurnManager.
 * - Stampa in log tutti gli attori presenti nella scena (per debug).
 * - Crea dinamicamente una telecamera dall'alto (top-down) e la imposta come attiva.
 * - Rimuove eventuali luci direzionali preesistenti e ne crea una nuova personalizzata.
 */
void AGridManager::BeginPlay()
{
    Super::BeginPlay(); // Chiama il BeginPlay della superclasse

    HighlightedTiles.Empty(); // Pulisce eventuali celle evidenziate in editor o da partite precedenti

    // DEBUG: stampa in log tutti gli attori presenti nel livello (utile per identificare collisioni o attori inutili)
    TArray<AActor*> BlockingActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), BlockingActors);
    for (AActor* Actor : BlockingActors)
    {
        UE_LOG(LogTemp, Warning, TEXT("Oggetto nel livello: %s"), *Actor->GetName());
    }

    // Recupera il GameMode corrente
    GameMode = Cast<AMyGameMode>(UGameplayStatics::GetGameMode(this));

    // Se il GameMode è valido, recupera anche il TurnManager da esso
    if (GameMode)
    {
        TurnManager = GameMode->GetTurnManager();
    }

    // Se non abbiamo trovato il TurnManager, mostriamo un errore
    if (!TurnManager)
    {
        UE_LOG(LogTemp, Error, TEXT("Errore: TurnManager non trovato!"));
    }

    // Creazione dinamica della camera di gioco (telecamera dall’alto)
    FActorSpawnParameters SpawnParams;
    ACameraActor* TopDownCamera = GetWorld()->SpawnActor<ACameraActor>(
        ACameraActor::StaticClass(), 
        FVector(1300, 1608, 2600),  // Posizione in alto centrata rispetto alla griglia
        FRotator(-90, 0, 0),       // Rotazione verso il basso (top-down puro)
        SpawnParams
    );

    // Configurazione della camera se è stata creata correttamente
    if (TopDownCamera)
    {
        // Ottiene il componente Camera per modificare impostazioni avanzate
        if (UCameraComponent* CamComp = Cast<UCameraComponent>(
            TopDownCamera->GetComponentByClass(UCameraComponent::StaticClass())))
        {
            // Disabilita il rapporto d’aspetto fisso per evitare bande nere su schermi larghi
            CamComp->SetConstraintAspectRatio(false);
        }

        // Imposta la telecamera appena creata come vista attiva per il giocatore
        if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0))
        {
            PlayerController->SetViewTarget(TopDownCamera);
        }
    }

    // Rimuove tutte le luci direzionali esistenti (predefinite della scena) per evitare conflitti visivi
    for (TActorIterator<ADirectionalLight> It(GetWorld()); It; ++It)
    {
        It->Destroy(); // Distrugge ogni luce direzionale trovata
    }

    // Crea una nuova luce direzionale (simula la luce del sole dall’alto)
    ADirectionalLight* DirectionalLight = GetWorld()->SpawnActor<ADirectionalLight>(
        ADirectionalLight::StaticClass(), 
        FVector(1000, 3000, 1000), // Posizionata sopra la scena
        FRotator(-90, 0, 0),       // Inclinata direttamente verso il basso
        SpawnParams
    );

    // Configura l’intensità, colore e comportamento della luce
    if (DirectionalLight)
    {
        if (ULightComponent* LightComp = DirectionalLight->GetLightComponent())
        {
            LightComp->SetMobility(EComponentMobility::Movable); // Può essere spostata dinamicamente
            LightComp->SetIntensity(3.5f);                        // Intensità luminosa
            LightComp->SetLightColor(FLinearColor(1.0f, 1.0f, 1.0f)); // Bianco puro
            LightComp->SetCastShadows(false);                    // Non proietta ombre (migliora la visibilità)
        }

        UE_LOG(LogTemp, Warning, TEXT("Directional Light creata con successo senza ombre!"));
    }

    // Log di conferma posizione iniziale del GridManager
    UE_LOG(LogTemp, Warning, TEXT("GridManagerCPP - Posizione griglia: %s"), *GetActorLocation().ToString());
}

/**
 * Genera dinamicamente una griglia 2D di Tile con dimensioni DimGridX x DimGridY.
 * Ogni cella viene posizionata nello spazio e identificata con un nome univoco (es. A1, B5...).
 * Prima della generazione, eventuali tile esistenti vengono distrutte.
 */
void AGridManager::GenerateGrid()
{
    // Prima di creare la nuova griglia, distruggiamo tutte le tile precedenti (se ancora valide)
    for (ATile* Tile : Grid)
    {
        if (IsValid(Tile))
        {
            Tile->Destroy(); // Rimuove l'attore dal mondo
        }
    }
    Grid.Empty(); // Svuota l’array per ripartire da zero

    // Ciclo annidato per righe e colonne della griglia
    for (int Row = 0; Row < DimGridY; Row++)
    {
        for (int Column = 0; Column < DimGridX; Column++)
        {
            // Genera un identificatore per la tile (es. A1, B2, ecc.)
            FString CellIdentifier = FString::Printf(TEXT("%c%d"), 'A' + Row, Column + 1);

            // Calcola la posizione spaziale della cella in base a dimensione e spaziatura
            FVector Location = FVector(Column * (CellSize + Spacing), Row * (CellSize + Spacing), 0);

            // Parametri per lo spawn della tile
            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            // Crea una nuova tile nel mondo
            ATile* Tile = GetWorld()->SpawnActor<ATile>(ATile::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);

            Tile->SetAsObstacle(true);                // La inizializziamo temporaneamente come ostacolo (verrà aggiornata dopo)
            Tile->SetTileIdentifier(CellIdentifier);  // Assegna un nome identificativo
            Grid.Add(Tile);                           // Aggiungi la tile alla lista della griglia
        }
    }

    // Log di conferma
    UE_LOG(LogTemp, Warning, TEXT(" Griglia generata con %d celle."), Grid.Num());
}

/**
 * Seleziona casualmente un sottoinsieme di tile da lasciare libere (non ostacoli).
 * Il numero totale di tile libere è determinato dalla percentuale ObstaclePercentage.
 * Utilizza la DFS per garantire che tutte le celle libere siano collegate tra loro.
 */
void AGridManager::GenerateObstacles()
{
    TSet<ATile*> Visited; // Tiene traccia delle tile già visitate dalla DFS
    int32 TotalObstacles = FMath::RoundToInt(Grid.Num() * ObstaclePercentage); // Quante tile saranno ostacoli

    // Inizia la DFS dalla prima tile (Grid[0]), solo se la griglia è stata generata
    if (Grid.Num() > 0)
    {
        DFS(Grid[0], Visited, TotalObstacles);
    }
}

/**
 * Algoritmo ricorsivo di visita in profondità (Depth-First Search) per attraversare le tile.
 * Lo scopo è "liberare" un numero sufficiente di tile (cioè rimuovere l'ostacolo impostandolo a false).
 * Tutte le tile visitate saranno collegate tra loro, evitando aree isolate.
 *
 * @param CurrentTile: La tile corrente su cui stiamo lavorando
 * @param Visited: Set delle tile già visitate
 * @param MaxObstacles: Numero massimo di ostacoli che devono rimanere nella griglia
 */
void AGridManager::DFS(ATile* CurrentTile, TSet<ATile*>& Visited, int32 MaxObstacles)
{
    // Condizione di terminazione: tile nulla, già visitata, o troppe celle libere
    if (!CurrentTile || Visited.Contains(CurrentTile) || Grid.Num() - Visited.Num() <= MaxObstacles)
    {
        return;
    }

    Visited.Add(CurrentTile);           // Segna la tile come visitata
    CurrentTile->SetAsObstacle(false);  // Rende la tile libera (non ostacolo)

    // Recupera le tile adiacenti
    TArray<ATile*> Neighbors = GetNeighbors(CurrentTile);

    // Mischia l'ordine dei vicini per rendere la DFS casuale
    for (int32 i = Neighbors.Num() - 1; i > 0; --i)
    {
        int32 j = FMath::RandRange(0, i);
        Neighbors.Swap(i, j); // Scambia i con un indice casuale j
    }

    // Chiama ricorsivamente DFS su ogni vicino
    for (ATile* Neighbor : Neighbors)
    {
        DFS(Neighbor, Visited, MaxObstacles);
    }
}

/**
 * Restituisce tutte le tile adiacenti (su, giù, sinistra, destra) alla tile specificata.
 * 
 * @param Tile: la tile di partenza
 * @return Lista di tile vicine nella griglia
 */
TArray<ATile*> AGridManager::GetNeighbors(ATile* Tile)
{
    TArray<ATile*> Neighbors;

    // Ottiene l'indice della tile nel vettore Grid
    int Index = Grid.Find(Tile);
    if (Index == INDEX_NONE) return Neighbors; // Tile non trovata → nessun vicino

    // Calcola la posizione della tile in coordinate di griglia
    int Row = Index / DimGridX;
    int Col = Index % DimGridX;

    // Controlla le 4 direzioni e aggiunge i vicini validi (senza uscire dai limiti della griglia)
    if (Row + 1 < DimGridY) Neighbors.Add(Grid[(Row + 1) * DimGridX + Col]);     // In basso
    if (Row - 1 >= 0)      Neighbors.Add(Grid[(Row - 1) * DimGridX + Col]);     // In alto
    if (Col + 1 < DimGridX) Neighbors.Add(Grid[Row * DimGridX + (Col + 1)]);    // A destra
    if (Col - 1 >= 0)      Neighbors.Add(Grid[Row * DimGridX + (Col - 1)]);     // A sinistra

    return Neighbors;
}

/**
 * Evidenzia la tile attualmente sotto l'unità passata come parametro.
 * Se c’era una tile precedentemente evidenziata (di un'altra unità), la resetta al colore normale.
 *
 * @param Unit: l’unità attualmente selezionata
 * @param Color: il colore con cui evidenziare la tile (es. arancione)
 */
void AGridManager::HighlightTileUnderUnit(AUnitBase* Unit, const FLinearColor& Color)
{
    if (!Unit) return; // Se l’unità non è valida, non facciamo nulla

    // Se una tile era precedentemente evidenziata, la ripristiniamo
    if (TileUnderSelectedUnit)
    {
        TileUnderSelectedUnit->SetHighlight(false, FLinearColor(0.498f, 0.498f, 0.498f, 1.0f));
        TileUnderSelectedUnit = nullptr;
    }

    // Trova la tile sotto la nuova unità selezionata
    TileUnderSelectedUnit = FindTileAtLocation(Unit->GetActorLocation());

    // Se trovata, applica l’evidenziazione con il colore desiderato
    if (TileUnderSelectedUnit)
    {
        TileUnderSelectedUnit->SetHighlight(true, Color);
    }
}

/**
 * Calcola e restituisce tutte le tile raggiungibili dall’unità selezionata
 * usando una BFS, tenendo conto del range di movimento.
 * Evita celle ostacolate o già occupate.
 *
 * @param SelectedUnit: l’unità che vuole muoversi
 * @return Array di tile valide per il movimento
 */
TArray<ATile*> AGridManager::GetValidMovementTiles(AUnitBase* SelectedUnit)
{
    TArray<ATile*> ValidTiles; // Risultato finale

    // Validazione iniziale: controlla che l’unità sia valida
    if (!SelectedUnit)
    {
        UE_LOG(LogTemp, Error, TEXT("GetValidMovementTiles: Unità selezionata nulla!"));
        return ValidTiles;
    }

    // Ottiene posizione e range di movimento dell’unità
    FVector UnitLocation = SelectedUnit->GetActorLocation();
    int32 MovementRange = SelectedUnit->GetMovementRange();

    // Trova la tile su cui si trova l'unità
    ATile* StartTile = FindTileAtLocation(UnitLocation);
    if (!StartTile)
    {
        UE_LOG(LogTemp, Error, TEXT("GetValidMovementTiles: Nessuna tile trovata sotto l'unità!"));
        return ValidTiles;
    }

    // Breadth-First Search (BFS) per esplorare le tile vicine
    TQueue<ATile*> Queue;             // Coda per BFS
    TMap<ATile*, int32> VisitedTiles; // Tiene traccia delle distanze

    Queue.Enqueue(StartTile);
    VisitedTiles.Add(StartTile, 0);

    while (!Queue.IsEmpty())
    {
        ATile* CurrentTile;
        Queue.Dequeue(CurrentTile);

        int32 CurrentDistance = VisitedTiles[CurrentTile];

        // Se superiamo il range dell’unità, saltiamo
        if (CurrentDistance >= MovementRange)
        {
            continue;
        }

        // Trova le tile vicine alla tile attuale
        TArray<ATile*> Neighbors = GetNeighbors(CurrentTile);

        for (ATile* Neighbor : Neighbors)
        {
            // Salta se la tile è ostacolo o contiene già una pedina
            if (Neighbor->IsObstacle() || Neighbor->GetHasPawn())
            {
                continue;
            }

            // Se la tile non è ancora stata visitata, la aggiungiamo
            if (!VisitedTiles.Contains(Neighbor))
            {
                Queue.Enqueue(Neighbor);
                VisitedTiles.Add(Neighbor, CurrentDistance + 1);
                ValidTiles.Add(Neighbor); // Aggiungiamo alle celle valide
            }
        }
    }

    return ValidTiles;
}

/**
 * Evidenzia le celle su cui l'unità può muoversi (quelle restituite da GetValidMovementTiles)
 * usando un colore blu per la visualizzazione.
 *
 * @param SelectedUnit: l’unità che vogliamo evidenziare
 */
void AGridManager::HighlightMovementTiles(AUnitBase* SelectedUnit)
{
    // Verifica se l’unità può ancora agire
    if (!SelectedUnit || !SelectedUnit->CanAct())
    {
        UE_LOG(LogTemp, Error, TEXT("HighlightMovementTiles: Nessuna unità selezionata."));
        return;
    }

    // Ottieni la lista delle tile valide per il movimento
    TArray<ATile*> ValidTiles = GetValidMovementTiles(SelectedUnit);

    // Evidenzia ciascuna tile con colore blu
    for (ATile* Tile : ValidTiles)
    {
        if (IsValid(Tile))
        {
            Tile->SetHighlight(true, FLinearColor(0.0f, 0.5f, 1.0f)); // Blu
            HighlightedTiles.Add(Tile); // Aggiungila alla lista delle tile evidenziate
        }
    }
}

/**
 * Rimuove l’evidenziazione da tutte le tile attualmente evidenziate
 * (sia per movimento che attacco) e resetta la tile sotto l’unità selezionata.
 */
void AGridManager::ClearHighlights()
{
    // Resetta tutte le tile evidenziate
    for (ATile* Tile : HighlightedTiles)
    {
        if (IsValid(Tile))
        {
            // Imposta il colore grigio di default
            Tile->SetHighlight(false, FLinearColor(0.498f, 0.498f, 0.498f, 1.0f));
        }
    }

    HighlightedTiles.Empty(); // Svuota la lista

    // Rimuove l’evidenziazione dalla tile sotto l’unità selezionata
    if (TileUnderSelectedUnit)
    {
        TileUnderSelectedUnit->SetHighlight(false, FLinearColor(0.498f, 0.498f, 0.498f, 1.0f));
        TileUnderSelectedUnit = nullptr;
    }
}

/**
 * Trova la tile più vicina alla posizione specificata (di solito quella sotto un’unità).
 * Usa una tolleranza di 50 unità per evitare problemi di precisione con il posizionamento.
 *
 * @param Location: posizione da controllare
 * @return puntatore alla tile trovata, o nullptr se nessuna tile corrisponde
 */
ATile* AGridManager::FindTileAtLocation(FVector Location)
{
    UE_LOG(LogTemp, Warning, TEXT("Cerco Tile alla posizione: X=%.1f, Y=%.1f"), Location.X, Location.Y);

    for (ATile* Tile : Grid)
    {
        if (FVector::Dist2D(Tile->GetActorLocation(), Location) < 50.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("Trovata Tile: %s"), *Tile->GetName());
            return Tile;
        }
    }

    UE_LOG(LogTemp, Error, TEXT("ERRORE: Nessuna Tile trovata alla posizione X=%.1f, Y=%.1f"), Location.X, Location.Y);
    return nullptr;
}

/**
 * Restituisce tutte le tile su cui l’unità può effettuare un attacco, tenendo conto del raggio d’attacco,
 * degli ostacoli (per i Brawler), e della presenza di nemici.
 *
 * @param Attacker: unità che sta attaccando
 * @return array di tile attaccabili
 */
TArray<ATile*> AGridManager::GetValidAttackTiles(AUnitBase* Attacker)
{
    TArray<ATile*> ValidTiles;

    if (!Attacker) return ValidTiles;

    FVector Origin = Attacker->GetActorLocation();
    int32 AttackRange = Attacker->GetAttackRange();
    bool bIsRanged = Attacker->IsRangedAttack();

    for (ATile* Tile : Grid)
    {
        if (!Tile) continue;

        float Distance = FVector::Dist2D(Tile->GetActorLocation(), Origin);

        // Verifica se la distanza rientra nel raggio d'attacco
        if (Distance <= AttackRange * (CellSize + Spacing))
        {
            // I Brawler non possono colpire attraverso ostacoli
            if (!bIsRanged && Tile->IsObstacle()) continue;

            // Cerchiamo un'unità nemica su quella tile
            for (TActorIterator<AUnitBase> It(GetWorld()); It; ++It)
            {
                AUnitBase* Target = *It;
                if (Target && Target != Attacker && Target->IsPlayerControlled() != Attacker->IsPlayerControlled())
                {
                    FVector TargetLocation = Target->GetActorLocation();
                    if (FVector::Dist2D(TargetLocation, Tile->GetActorLocation()) < 50.f)
                    {
                        ValidTiles.Add(Tile);
                        break;
                    }
                }
            }
        }
    }

    return ValidTiles;
}

void AGridManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

/**
 * Calcola un percorso valido dalla tile dell’unità alla destinazione usando BFS (Breadth-First Search).
 * Evita celle bloccate da ostacoli o occupate, eccetto se la destinazione stessa è occupata.
 *
 * @param Unit: unità che vuole muoversi
 * @param Destination: tile da raggiungere
 * @return array di tile che formano il percorso, ordinato dalla partenza alla destinazione
 */
TArray<ATile*> AGridManager::GetPathToTile(AUnitBase* Unit, ATile* Destination)
{
    UE_LOG(LogTemp, Warning, TEXT("Calcolo percorso per %s da X=%.1f, Y=%.1f a X=%.1f, Y=%.1f"),
        *Unit->GetName(),
        Unit->GetActorLocation().X, Unit->GetActorLocation().Y,
        Destination->GetActorLocation().X, Destination->GetActorLocation().Y);

    TArray<ATile*> Path;
    if (!Unit || !Destination) return Path;

    ATile* StartTile = FindTileAtLocation(Unit->GetActorLocation());
    if (!StartTile)
    {
        UE_LOG(LogTemp, Error, TEXT("ERRORE: Nessuna tile iniziale trovata per %s!"), *Unit->GetName());
        return Path;
    }

    TMap<ATile*, ATile*> CameFrom;
    TQueue<ATile*> Queue;
    TSet<ATile*> Visited;

    Queue.Enqueue(StartTile);
    Visited.Add(StartTile);

    bool bPathFound = false;

    while (!Queue.IsEmpty())
    {
        ATile* Current;
        Queue.Dequeue(Current);

        if (Current == Destination)
        {
            bPathFound = true;
            break;
        }

        for (ATile* Neighbor : GetNeighbors(Current))
        {
            bool bIsFinalDestination = (Neighbor == Destination);
            bool bIsOccupied = Neighbor->GetHasPawn();
            bool bIsObstacle = Neighbor->IsObstacle();

            if (!Visited.Contains(Neighbor) && !bIsObstacle && (!bIsOccupied || bIsFinalDestination))
            {
                Visited.Add(Neighbor);
                CameFrom.Add(Neighbor, Current);
                Queue.Enqueue(Neighbor);
            }
        }
    }

    if (!bPathFound)
    {
        UE_LOG(LogTemp, Error, TEXT("ERRORE: Nessun percorso trovato per %s!"), *Unit->GetName());
        return Path;
    }

    // Ricostruisce il percorso andando a ritroso
    ATile* Current = Destination;
    while (Current && CameFrom.Contains(Current))
    {
        Path.Insert(Current, 0);
        Current = CameFrom[Current];
    }

    UE_LOG(LogTemp, Warning, TEXT("Percorso trovato con %d passi"), Path.Num());
    return Path;
}

/**
 * Finalizza il movimento dell’unità aggiornando lo stato delle tile (occupata/non occupata)
 * e posizionando l’unità esattamente sulla nuova tile.
 *
 * @param Unit: unità che ha terminato il movimento
 * @param DestinationTile: tile di destinazione su cui posizionarla
 */
void AGridManager::FinalizeUnitMovement(AUnitBase* Unit, ATile* DestinationTile)
{
    if (!Unit || !DestinationTile) return;

    // Libera la vecchia tile
    ATile* OldTile = FindTileAtLocation(Unit->GetActorLocation());
    if (OldTile)
    {
        OldTile->SetHasPawn(false);
    }

    // Sposta l’unità nella nuova tile
    Unit->SetActorLocation(DestinationTile->GetPawnSpawnLocation());

    // Segna la nuova tile come occupata
    DestinationTile->SetHasPawn(true);
}

/**
 * Evidenzia visivamente tutte le tile attaccabili da una specifica unità.
 * Le celle valide sono calcolate tramite `GetValidAttackTiles()` e colorate in rosso.
 *
 * @param AttackingUnit: unità che intende attaccare
 */
void AGridManager::HighlightAttackGrid(AUnitBase* AttackingUnit)
{
    // L'unità non può attaccare se ha già attaccato
    if (!AttackingUnit || AttackingUnit->GetCurrentAction() == EUnitAction::Attacked || AttackingUnit->GetCurrentAction() == EUnitAction::MoveAttack)
        return;

    AttackGridTiles = GetValidAttackTiles(AttackingUnit);

    for (ATile* Tile : AttackGridTiles)
    {
        Tile->SetHighlight(true, FLinearColor::Red);
        HighlightedTiles.Add(Tile);
    }
}

/**
 * Cerca e restituisce l’unità presente su una determinata tile.
 * Confronta la posizione delle unità con quella della tile.
 *
 * @param Tile: tile su cui cercare
 * @return puntatore all’unità trovata (se presente), altrimenti nullptr
 */
AUnitBase* AGridManager::GetUnitOnTile(ATile* Tile) const
{
    if (!Tile) return nullptr;

    for (TActorIterator<AUnitBase> It(GetWorld()); It; ++It)
    {
        AUnitBase* Unit = *It;
        if (Unit && FVector::Dist2D(Unit->GetActorLocation(), Tile->GetActorLocation()) < 50.f)
        {
            return Unit;
        }
    }

    return nullptr;
}