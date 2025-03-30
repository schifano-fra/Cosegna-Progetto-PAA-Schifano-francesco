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

AGridManager::AGridManager()
{
    PrimaryActorTick.bCanEverTick = false;
    CurrentSelectedUnit = nullptr;
    TurnManager = nullptr;
    bAttackGridVisible = false;
    float RandomPercent = FMath::FRandRange(0.3f, 0.95f);
    ObstaclePercentage = FMath::Clamp(RandomPercent, 0.3f, 0.95f);
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

void AGridManager::BeginPlay()
{
    Super::BeginPlay();
    HighlightedTiles.Empty();
    TArray<AActor*> BlockingActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), BlockingActors);
    for (AActor* Actor : BlockingActors)
    {
        UE_LOG(LogTemp, Warning, TEXT("Oggetto nel livello: %s"), *Actor->GetName());
    }
    
    GameMode = Cast<AMyGameMode>(UGameplayStatics::GetGameMode(this));

    if (GameMode)
    {
        TurnManager = GameMode->GetTurnManager();
    }
    
    if (!TurnManager)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Errore: TurnManager non trovato!"));
    }
    
    // Creazione dinamica della telecamera
    FActorSpawnParameters SpawnParams;
    // Sposta la camera più in alto e con un’angolazione minore, così hai spazio sopra la griglia
    ACameraActor* TopDownCamera = GetWorld()->SpawnActor<ACameraActor>(
        ACameraActor::StaticClass(), 
        FVector(1300, 1608, 2600),   // +500 in altezza rispetto a prima
        FRotator(-90, 0, 0),        // inclinazione ridotta (invece di -90)
        SpawnParams
    );

    if (TopDownCamera)
    {
        // Disabilita l'aspect ratio fisso per rimuovere le bande nere
        if (UCameraComponent* CamComp = Cast<UCameraComponent>(
            TopDownCamera->GetComponentByClass(UCameraComponent::StaticClass())))
        {
            CamComp->SetConstraintAspectRatio(false);
            // Puoi anche regolare CamComp->SetAspectRatio(...) se vuoi un aspetto specifico
        }

        if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0))
        {
            PlayerController->SetViewTarget(TopDownCamera);
        }
    }

    // (Opzionale) Ripristina la distruzione di eventuali luci direzionali esistenti
    for (TActorIterator<ADirectionalLight> It(GetWorld()); It; ++It)
    {
        It->Destroy();
    }

    // Creazione dinamica di una vera luce direzionale (non un'altra camera!)
    ADirectionalLight* DirectionalLight = GetWorld()->SpawnActor<ADirectionalLight>(
        ADirectionalLight::StaticClass(), 
        FVector(1000, 3000, 1000), 
        FRotator(-90, 0, 0), 
        SpawnParams
    );
    if (DirectionalLight)
    {
        if (ULightComponent* LightComp = DirectionalLight->GetLightComponent())
        {
            LightComp->SetMobility(EComponentMobility::Movable);
            LightComp->SetIntensity(3.5f);
            LightComp->SetLightColor(FLinearColor(1.0f, 1.0f, 1.0f));
            LightComp->SetCastShadows(false);
        }
        UE_LOG(LogTemp, Warning, TEXT("Directional Light creata con successo senza ombre!"));
    }

    UE_LOG(LogTemp, Warning, TEXT("GridManagerCPP - Posizione griglia: %s"), *GetActorLocation().ToString());;
}

void AGridManager::GenerateGrid()
{
    // 🔄 Distruggi le vecchie tile prima di svuotare l’array
    for (ATile* Tile : Grid)
    {
        if (IsValid(Tile))
        {
            Tile->Destroy();
        }
    }
    Grid.Empty();

    for (int Row = 0; Row < DimGridY; Row++)
    {
        for (int Column = 0; Column < DimGridX; Column++)
        {
            // Calcoliamo la lettera (A-Y) e il numero (1-25)
            FString CellIdentifier = FString::Printf(TEXT("%c%d"), 'A' + Column, Row + 1); // A1, B1, C1...

            FVector Location = FVector(Column * (CellSize + Spacing), Row * (CellSize + Spacing), 0);

            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            ATile* Tile = GetWorld()->SpawnActor<ATile>(ATile::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
            Tile->SetAsObstacle(true);
            Tile->SetTileIdentifier(CellIdentifier);  // Assegniamo l'identificativo alla cella
            Grid.Add(Tile);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("✅ Griglia generata con %d celle."), Grid.Num());
}

void AGridManager::GenerateObstacles()
{
    TSet<ATile*> Visited;
    int32 TotalObstacles = FMath::RoundToInt(Grid.Num() * ObstaclePercentage);

    if (Grid.Num() > 0)
    {
        DFS(Grid[0], Visited, TotalObstacles);
    }
}

void AGridManager::DFS(ATile* CurrentTile, TSet<ATile*>& Visited, int32 MaxObstacles)
{
    if (!CurrentTile || Visited.Contains(CurrentTile) || Grid.Num()-Visited.Num() <= MaxObstacles)
    {
        return;
    }

    Visited.Add(CurrentTile);
    CurrentTile->SetAsObstacle(false);

    TArray<ATile*> Neighbors = GetNeighbors(CurrentTile);

    for (int32 i = Neighbors.Num() - 1; i > 0; --i)
    {
        int32 j = FMath::RandRange(0, i);
        Neighbors.Swap(i, j);
    }

    for (ATile* Neighbor : Neighbors)
    {
        DFS(Neighbor, Visited, MaxObstacles);
    }
}

TArray<ATile*> AGridManager::GetNeighbors(ATile* Tile)
{
    TArray<ATile*> Neighbors;

    int Index = Grid.Find(Tile);
    if (Index == INDEX_NONE) return Neighbors;

    int Row = Index / DimGridX;
    int Col = Index % DimGridX;

    if (Row + 1 < DimGridY) Neighbors.Add(Grid[(Row + 1) * DimGridX + Col]);
    if (Row - 1 >= 0) Neighbors.Add(Grid[(Row - 1) * DimGridX + Col]);
    if (Col + 1 < DimGridX) Neighbors.Add(Grid[Row * DimGridX + (Col + 1)]);
    if (Col - 1 >= 0) Neighbors.Add(Grid[Row * DimGridX + (Col - 1)]);

    return Neighbors;
}

void AGridManager::HighlightTileUnderUnit(AUnitBase* Unit, const FLinearColor& Color)
{
    if (!Unit) return;

    if (TileUnderSelectedUnit)
    {
        TileUnderSelectedUnit->SetHighlight(false, FLinearColor(0.498f, 0.498f, 0.498f, 1.0f));
        TileUnderSelectedUnit = nullptr;
    }

    TileUnderSelectedUnit = FindTileAtLocation(Unit->GetActorLocation());

    if (TileUnderSelectedUnit)
    {
        TileUnderSelectedUnit->SetHighlight(true, Color);
    }
}

TArray<ATile*> AGridManager::GetValidMovementTiles(AUnitBase* SelectedUnit)
{
    TArray<ATile*> ValidTiles;

    if (!SelectedUnit)
    {
        UE_LOG(LogTemp, Error, TEXT("GetValidMovementTiles: Unità selezionata nulla!"));
        return ValidTiles;
    }

    FVector UnitLocation = SelectedUnit->GetActorLocation();
    int32 MovementRange = SelectedUnit->GetMovementRange(); // 3 per Sniper, 6 per Brawler

    // Trova la Tile su cui si trova attualmente l'unità
    ATile* StartTile = FindTileAtLocation(UnitLocation);
    if (!StartTile)
    {
        UE_LOG(LogTemp, Error, TEXT("GetValidMovementTiles: Nessuna tile trovata sotto l'unità!"));
        return ValidTiles;
    }

    // BFS per trovare le celle valide
    TQueue<ATile*> Queue;
    TMap<ATile*, int32> VisitedTiles;

    Queue.Enqueue(StartTile);
    VisitedTiles.Add(StartTile, 0);

    while (!Queue.IsEmpty())
    {
        ATile* CurrentTile;
        Queue.Dequeue(CurrentTile);

        int32 CurrentDistance = VisitedTiles[CurrentTile];

        // Se superiamo il range di movimento, fermiamo l'esplorazione
        if (CurrentDistance >= MovementRange)
        {
            continue;
        }

        // Troviamo le celle adiacenti
        TArray<ATile*> Neighbors = GetNeighbors(CurrentTile);

        for (ATile* Neighbor : Neighbors)
        {
            // Escludiamo ostacoli e celle già occupate
            if (Neighbor->IsObstacle() || Neighbor->GetHasPawn())
            {
                continue;
            }

            // Se la cella non è ancora stata visitata o può essere raggiunta con meno passi, la esploriamo
            if (!VisitedTiles.Contains(Neighbor))
            {
                Queue.Enqueue(Neighbor);
                VisitedTiles.Add(Neighbor, CurrentDistance + 1);
                ValidTiles.Add(Neighbor);
            }
        }
    }

    return ValidTiles;
}

// Evidenzia le celle di movimento disponibili in blu
void AGridManager::HighlightMovementTiles(AUnitBase* SelectedUnit)
{
    if (!SelectedUnit || !SelectedUnit->CanAct())
    {
        UE_LOG(LogTemp, Error, TEXT("HighlightMovementTiles: Nessuna unità selezionata."));
        return;
    }

    TArray<ATile*> ValidTiles = GetValidMovementTiles(SelectedUnit);

    for (ATile* Tile : ValidTiles)
    {
        if (IsValid(Tile))
        {
            Tile->SetHighlight(true, FLinearColor(0.0f, 0.5f, 1.0f)); // Blu
            HighlightedTiles.Add(Tile);
        }
    }
}

// Rimuove la colorazione dalle celle
void AGridManager::ClearHighlights()
{
    for (ATile* Tile : HighlightedTiles)
    {
        if (IsValid(Tile))
        {
            Tile->SetHighlight(false, FLinearColor(0.498f, 0.498f, 0.498f, 1.0f));
        }
    }

    HighlightedTiles.Empty();

    if (TileUnderSelectedUnit)
    {
        TileUnderSelectedUnit->SetHighlight(false, FLinearColor(0.498f, 0.498f, 0.498f, 1.0f));
        TileUnderSelectedUnit = nullptr;
    }
}

// Trova la tile corrispondente alla posizione dell'unità
ATile* AGridManager::FindTileAtLocation(FVector Location)
{
    UE_LOG(LogTemp, Warning, TEXT("🔎 Cerco Tile alla posizione: X=%.1f, Y=%.1f"), Location.X, Location.Y);

    for (ATile* Tile : Grid)
    {
        if (FVector::Dist2D(Tile->GetActorLocation(), Location) < 50.0f) // Tolleranza di 50 unità
        {
            UE_LOG(LogTemp, Warning, TEXT("✅ Trovata Tile: %s"), *Tile->GetName());
            return Tile;
        }
    }

    UE_LOG(LogTemp, Error, TEXT("❌ ERRORE: Nessuna Tile trovata alla posizione X=%.1f, Y=%.1f"), Location.X, Location.Y);
    return nullptr;
}

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
        if (Distance <= AttackRange * (CellSize + Spacing))
        {
            if (!bIsRanged && Tile->IsObstacle())
            {
                continue; // Brawler non può attaccare attraverso ostacoli
            }

            // Verifica se c’è un’unità nemica sulla tile
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

TArray<ATile*> AGridManager::GetPathToTile(AUnitBase* Unit, ATile* Destination)
{
    UE_LOG(LogTemp, Warning, TEXT("🔎 Calcolo percorso per %s da X=%.1f, Y=%.1f a X=%.1f, Y=%.1f"),
        *Unit->GetName(),
        Unit->GetActorLocation().X, Unit->GetActorLocation().Y,
        Destination->GetActorLocation().X, Destination->GetActorLocation().Y);

    TArray<ATile*> Path;
    if (!Unit || !Destination) return Path;

    ATile* StartTile = FindTileAtLocation(Unit->GetActorLocation());
    if (!StartTile)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ ERRORE: Nessuna tile iniziale trovata per %s!"), *Unit->GetName());
        return Path;
    }

    UE_LOG(LogTemp, Warning, TEXT("🔍 Parto da Tile %s"), *StartTile->GetName());

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

            /*UE_LOG(LogTemp, Warning, TEXT("🔍 Controllo Tile %s - Occupata: %s, Ostacolo: %s"),
                *Neighbor->GetName(),
                bIsOccupied ? TEXT("SI") : TEXT("NO"),
                bIsObstacle ? TEXT("SI") : TEXT("NO"));*/

            // Valida se non visitata, non ostacolo, e non occupata — a meno che sia la destinazione
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
        UE_LOG(LogTemp, Error, TEXT("❌ ERRORE: Nessun percorso trovato per %s!"), *Unit->GetName());
        return Path;
    }

    // Ricostruzione del percorso
    ATile* Current = Destination;
    while (Current && CameFrom.Contains(Current))
    {
        Path.Insert(Current, 0);
        Current = CameFrom[Current];
    }

    UE_LOG(LogTemp, Warning, TEXT("✅ Percorso trovato con %d passi"), Path.Num());
    return Path;
}

void AGridManager::FinalizeUnitMovement(AUnitBase* Unit, ATile* DestinationTile)
{
    if (!Unit || !DestinationTile) return;

    // Trova la vecchia tile dell’unità
    ATile* OldTile = FindTileAtLocation(Unit->GetActorLocation());
    if (OldTile)
    {
        OldTile->SetHasPawn(false);
    }

    // Aggiorna la posizione dell'unità
    Unit->SetActorLocation(DestinationTile->GetPawnSpawnLocation());

    // Segna la nuova tile come occupata
    DestinationTile->SetHasPawn(true);
}

void AGridManager::HighlightAttackGrid(AUnitBase* AttackingUnit)
{
    if (!AttackingUnit || AttackingUnit->GetCurrentAction() == EUnitAction::Attacked || AttackingUnit->GetCurrentAction() == EUnitAction::MoveAttack) return;

    AttackGridTiles = GetValidAttackTiles(AttackingUnit);
    for (ATile* Tile : AttackGridTiles)
    {
        Tile->SetHighlight(true, FLinearColor::Red);
        HighlightedTiles.Add(Tile);
    }
}

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