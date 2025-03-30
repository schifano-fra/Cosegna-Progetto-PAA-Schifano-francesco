#include "PlacementManager.h"
#include "MyGameMode.h"
#include "TurnManager.h"
#include "PAASchifanoFrancesco/Units/Brawler.h"
#include "PAASchifanoFrancesco/Units/Sniper.h"
#include "PAASchifanoFrancesco/Grid/GridManager.h"
#include "PAASchifanoFrancesco/Grid/Tile.h"
#include "PAASchifanoFrancesco/UI/SelectPawn.h"
#include "Kismet/GameplayStatics.h"

APlacementManager::APlacementManager()
{
    PrimaryActorTick.bCanEverTick = false;
    GridManager = nullptr;
}

void APlacementManager::Initialize(TSubclassOf<USelectPawn> InSelectPawnClass)
{
    SelectPawnClass = InSelectPawnClass;
    GM = Cast<AMyGameMode>(UGameplayStatics::GetGameMode(this));
}

void APlacementManager::StartPlacement()
{
    GridManager = GM->GetGridManager();

    if (SelectPawnClass)
    {
        SelectPawn = CreateWidget<USelectPawn>(GetWorld(), SelectPawnClass);

        if (SelectPawn)
        {
            SelectPawn->AddToViewport();
        }
    }
    
    GM->TurnManager->StartTurn();
}

void APlacementManager::HandleTileClick(ATile* ClickedTile)
{
    if (!ClickedTile || !PlayerPawnType)
    {
        UE_LOG(LogTemp, Error, TEXT("HandleTileClick: Missing tile or pawn type"));
        return;
    }
    
    if (!GM || !GM->TurnManager) return;
    
    if (GM->GetCurrentGamePhase() != EGamePhase::EPlacement)
    {
        UE_LOG(LogTemp, Warning, TEXT("Click ignorato: non siamo nella fase di piazzamento."));
        return;
    }

    if (GM->TurnManager->GetCurrentPlayer() == EPlayer::Player1)
    {
        UE_LOG(LogTemp, Warning, TEXT("🧩 Tentativo di piazzare un'unità di tipo: %s"), *PlayerPawnType->GetName());

        PlacePlayerPawn(ClickedTile);
    }
}

void APlacementManager::FinishPlacementPhase()
{
    if (SelectPawn)
    {
        SelectPawn->RemoveFromParent();
        SelectPawn = nullptr;
    }
    
    if (GM)
    {
        GM->SetGamePhase(EGamePhase::EBattle);
    }
}

void APlacementManager::PlacePlayerPawn(ATile* ClickedTile)
{
    if (!PlayerPawnType || ClickedTile->IsObstacle() || ClickedTile->GetHasPawn())
    {
        UE_LOG(LogTemp, Error, TEXT("PlacePlayerPawn: SelectedPawnType is null."));
        return;
    }
    if (ClickedTile->IsObstacle() || ClickedTile->GetHasPawn())
    {
        UE_LOG(LogTemp, Error, TEXT("Cella già occupata"));
        return;
    }

    FVector SpawnLocation = ClickedTile->GetPawnSpawnLocation();
    UE_LOG(LogTemp, Warning, TEXT("📍 Posizione di spawn calcolata: %s"), *SpawnLocation.ToString());
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    SpawnParams.bNoFail = true;

    AUnitBase* NewPawn = GetWorld()->SpawnActor<AUnitBase>(PlayerPawnType, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
    if (!NewPawn)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ SpawnActor ha fallito per: %s"), *PlayerPawnType->GetName());
    }
    if (NewPawn->IsA(ASniper::StaticClass()))
    {
        NewPawn->UnitDisplayName = TEXT("Sniper(Player)");
    }
    else if (NewPawn->IsA(ABrawler::StaticClass()))
    {
        NewPawn->UnitDisplayName = TEXT("Brawler(Player)");
    }
    ClickedTile->SetHasPawn(true);
    NewPawn->SetIsPlayerController(true);
    GM->TurnManager->RegisterPlacementMove(NewPawn);
    if (SelectPawn)
    {
        SelectPawn->DisableButtonForPawn(PlayerPawnType);
    }
    UE_LOG(LogTemp, Warning, TEXT("Player Pawn Placed: %s at %s"), *NewPawn->GetName(), *SpawnLocation.ToString());
    PlayerPawnType = nullptr;
}

void APlacementManager::PlaceAIPawn()
{
    if (!GridManager)
    {
        UE_LOG(LogTemp, Error, TEXT("PlaceAIPawn: GridManager non valido!"));
        return;
    }

    TArray<ATile*> AvailableTiles;

    for (ATile* Tile : GridManager->GetGridTiles())
    {
        if (!Tile->IsObstacle() && !Tile->GetHasPawn())
        {
            AvailableTiles.Add(Tile);
        }
    }

    if (AvailableTiles.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlaceAIPawn: Nessuna Tile disponibile per posizionare l'IA."));
        return;
    }

    int32 RandomIndex = FMath::RandRange(0, AvailableTiles.Num() - 1);
    ATile* ChosenTile = AvailableTiles[RandomIndex];

    if (!ChosenTile)
    {
        UE_LOG(LogTemp, Error, TEXT("PlaceAIPawn: La Tile selezionata è nulla!"));
        return;
    }

    FVector SpawnLocation = ChosenTile->GetPawnSpawnLocation();
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AIPawnType = (GM->AIUnits.Num() == 0) ? ASniper::StaticClass() : ABrawler::StaticClass();

    AUnitBase* AIPawn = GetWorld()->SpawnActor<AUnitBase>(AIPawnType, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
    if (AIPawn)
    {
        if (AIPawn->IsA(ASniper::StaticClass()))
        {
            AIPawn->UnitDisplayName = TEXT("Sniper (AI)");
        }
        else if (AIPawn->IsA(ABrawler::StaticClass()))
        {
            AIPawn->UnitDisplayName = TEXT("Brawler (AI)");
        }
        ChosenTile->SetHasPawn(true);
        GM->TurnManager->RegisterPlacementMove(AIPawn);
        UE_LOG(LogTemp, Warning, TEXT("AI Pawn Placed: %s at %s"), *AIPawn->GetName(), *SpawnLocation.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PlaceAIPawn: Fallito il posizionamento del Pawn IA!"));
    }
}

void APlacementManager::SetSelectedPawnType(TSubclassOf<AUnitBase> PawnType, EPlayer Player)
{
    if (Player == EPlayer::Player1)
    {
        PlayerPawnType = PawnType;
    }
    else if (Player == EPlayer::AI)
    {
        AIPawnType = PawnType;
    }

    UE_LOG(LogTemp, Warning, TEXT("SelectedPawnType set to: %s for player %s"), *PawnType->GetName(), *UEnum::GetValueAsString(Player));
}
