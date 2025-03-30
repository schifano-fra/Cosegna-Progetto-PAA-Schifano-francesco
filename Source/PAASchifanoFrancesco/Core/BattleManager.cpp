// Fill out your copyright notice in the Description page of Project Settings.

#include "BattleManager.h"
#include "MyGameMode.h"
#include "PAASchifanoFrancesco/Units/UnitBase.h"
#include "PAASchifanoFrancesco/Grid/GridManager.h"
#include "PAASchifanoFrancesco/Grid/Tile.h"
#include "PAASchifanoFrancesco/Units/UnitMovementManager.h"
#include "PAASchifanoFrancesco/UI/StatusGameWidget.h"
#include "Kismet/GameplayStatics.h"



ABattleManager::ABattleManager()
{
    PrimaryActorTick.bCanEverTick = false;
    SelectedUnit = nullptr;
    TurnManager = nullptr;
}

void ABattleManager::StartBattle()
{
    GameMode = Cast<AMyGameMode>(UGameplayStatics::GetGameMode(this));
    
    if (GameMode)
    {
        GridManager = GameMode->GetGridManager();
        TurnManager = GameMode->GetTurnManager();
        MovementManager = GameMode->GlobalMovementManager;
    }
    
    // **Assicuriamoci che il MovementManager esista**
    if (!MovementManager)
    {
        MovementManager = GameMode->GlobalMovementManager;
        if (!MovementManager)
        {
            UE_LOG(LogTemp, Error, TEXT("MovementManager NON TROVATO in BattleManager!"));
            return;
        }
        MovementManager->OnMovementFinished.AddDynamic(this, &ABattleManager::OnAIMovementComplete);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Battaglia iniziata. Primo turno: %s"), *UEnum::GetValueAsString(GameMode->TurnManager->GetCurrentPlayer()));

    GameMode->TurnManager->StartTurn();

    if (GameMode && GameMode->GetStatusGameWidget())
    {
        for (AUnitBase* Unit : GameMode->PlayerUnits)
        {
            GameMode->GetStatusGameWidget()->AddUnitStatus(Unit);
        }
        for (AUnitBase* Unit : GameMode->AIUnits)
        {
            GameMode->GetStatusGameWidget()->AddUnitStatus(Unit);
        }
    }
}

void ABattleManager::OnAIMovementComplete(AUnitBase* UnitBase)
{
    GridManager->ClearHighlights();
}

void ABattleManager::PrepareAITurn()
{
    if (!GameMode || !TurnManager || !GridManager) return;

    UE_LOG(LogTemp, Warning, TEXT("AI inizia il suo turno..."));

    AIUnitsToProcess = GameMode->AIUnits;
    CurrentAIIndex = 0;

    ProcessNextAIUnit();  // Inizia la gestione unità per unità
}

void ABattleManager::ProcessNextAIUnit()
{
    if (CurrentAIIndex >= AIUnitsToProcess.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("Tutte le unità AI hanno agito. Fine turno AI."));
        TurnManager->EndTurn();
        return;
    }

    AUnitBase* CurrentUnit = AIUnitsToProcess[CurrentAIIndex];
    if (!CurrentUnit || !CurrentUnit->CanAct() || CurrentUnit->GetCurrentAction() == EUnitAction::Attacked)
    {
        CurrentAIIndex++;
        ProcessNextAIUnit();
        return;
    }

    // 1. Mostra griglia movimento
    GridManager->HighlightMovementTiles(CurrentUnit);
    FTimerHandle MovementTimerHandle;
    GameMode->GetWorld()->GetTimerManager().SetTimer(MovementTimerHandle, [this, CurrentUnit]()
    {
        GridManager->ClearHighlights();

        // 2. Mostra griglia attacco
        GridManager->HighlightAttackGrid(CurrentUnit);
        FTimerHandle AttackTimerHandle;
        GameMode->GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, [this, CurrentUnit]()
        {
            GridManager->ClearHighlights();

            if (GameMode && GameMode->AILevel == EAILevel::Easy)
            {
                int32 Choice = FMath::RandRange(0, 1); // 0 = move, 1 = move+attack

                TryAIRandomMove(CurrentUnit);

                if (Choice == 1)
                {
                    // Attende 1 secondo e poi prova ad attaccare
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

                    return; // ✅ BLOCCA qui
                }
            }
            if (!TryAIAttack(CurrentUnit))
            {
                // Non ha attaccato → prova a muoversi
                TryAIMove(CurrentUnit);

                // Dopo 1s → riprova ad attaccare
                FTimerHandle AfterMoveHandle;
                GameMode->GetWorld()->GetTimerManager().SetTimer(AfterMoveHandle, [this, CurrentUnit]()
                {
                    GridManager->HighlightAttackGrid(CurrentUnit);
                    TryAIAttack(CurrentUnit);
                    FTimerHandle AttackTimer;
                    GameMode->GetWorld()->GetTimerManager().SetTimer(AttackTimer, [this, CurrentUnit]()
                    {
                        GridManager->ClearHighlights();
                        CurrentAIIndex++;
                        ProcessNextAIUnit();
                    }, 1.0f, false);
                    
                }, 5.0f, false);
            }
            else
            {
                CurrentAIIndex++;
                ProcessNextAIUnit();
            }
        }, 3.0f, false); // Tempo visibile per attacco

    }, 3.0f, false); // Tempo visibile per movimento
}

void ABattleManager::TryAIMove(AUnitBase* AIUnit)
{
    if (!AIUnit || !GridManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("TryAIMove: AIUnit o GridManager è nullo"));
        return;
    }

    AUnitBase* Enemy = FindNearestEnemy(AIUnit);
    if (!Enemy)
    {
        UE_LOG(LogTemp, Warning, TEXT("TryAIMove: Nessun nemico trovato"));
        return;
    }

    ATile* EnemyTile = GridManager->FindTileAtLocation(Enemy->GetActorLocation());
    if (!EnemyTile)
    {
        UE_LOG(LogTemp, Warning, TEXT("TryAIMove: EnemyTile non trovato"));
        return;
    }

    TArray<ATile*> Path = GridManager->GetPathToTile(AIUnit, EnemyTile);
    TArray<ATile*> Area = GridManager->GetValidMovementTiles(AIUnit);

    int32 MaxSteps = AIUnit->GetMovementRange();
    
    int32 LastReachableIndex = -1;

    for (int32 i = FMath::Min(MaxSteps - 1, Path.Num() - 1); i >= 0; i--)
    {
        if (Area.Contains(Path[i]))
        {
            LastReachableIndex = i;
            break;
        }
    }
    
    if (LastReachableIndex != -1)
    {
        TArray<ATile*> PathToMove;
        for (int32 i = 0; i <= LastReachableIndex; i++)
        {
            PathToMove.Add(Path[i]);
        }

        MovementManager->MoveUnit(AIUnit, PathToMove, 300.f);

        ATile* From = GridManager->FindTileAtLocation(AIUnit->GetActorLocation());
        FString FromName = From ? From->GetTileIdentifier() : TEXT("???");
        FString ToName = PathToMove.Last()->GetTileIdentifier();
        FString UnitType = AIUnit->IsRangedAttack() ? TEXT("Sniper") : TEXT("Brawler");
        GameMode->AddMoveToHistory(FString::Printf(TEXT("AI: %s moves from %s to %s"), *UnitType, *FromName, *ToName));

        TurnManager->RegisterAIMove(AIUnit);

        UE_LOG(LogTemp, Warning, TEXT("AI %s si muove verso %s per %d passi"),
            *AIUnit->GetName(),
            *Enemy->GetName(),
            PathToMove.Num());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Nessuna tile raggiungibile trovata nel path."));
    }
}

void ABattleManager::TryAIRandomMove(AUnitBase* AIUnit)
{
    if (!AIUnit || !GridManager) return;

    TArray<ATile*> MovableTiles = GridManager->GetValidMovementTiles(AIUnit);
    if (MovableTiles.Num() == 0) return;

    int32 Index = FMath::RandRange(1, AIUnit->GetMovementRange());
    ATile* Destination = MovableTiles[Index];

    TArray<ATile*> Path = GridManager->GetPathToTile(AIUnit, Destination);
    MovementManager->MoveUnit(AIUnit, Path, 300.f);

    FString From = GridManager->FindTileAtLocation(AIUnit->GetActorLocation()) ?
        GridManager->FindTileAtLocation(AIUnit->GetActorLocation())->GetTileIdentifier() : TEXT("???");

    FString To = Destination ? Destination->GetTileIdentifier() : TEXT("???");
    FString UnitType = AIUnit->IsRangedAttack() ? TEXT("Sniper") : TEXT("Brawler");

    GameMode->AddMoveToHistory(FString::Printf(TEXT("AI (Easy): %s moves from %s to %s"), *UnitType, *From, *To));
    TurnManager->RegisterAIMove(AIUnit);
}

bool ABattleManager::TryAIAttack(AUnitBase* AIUnit)
{
    TArray<ATile*> AttackTiles = GridManager->GetValidAttackTiles(AIUnit);

    for (AUnitBase* PlayerUnit : GameMode->PlayerUnits)
    {
        ATile* PlayerTile = GridManager->FindTileAtLocation(PlayerUnit->GetActorLocation());

        if (!PlayerTile) continue;

        if (AttackTiles.Contains(PlayerTile))
        {
            UE_LOG(LogTemp, Warning, TEXT("AI %s attacca %s"), *AIUnit->GetName(), *PlayerUnit->GetName());
            AIUnit->AttackUnit(PlayerUnit);
            
            ATile* TargetTile = GridManager->FindTileAtLocation(PlayerUnit->GetActorLocation());
            FString TileName = TargetTile ? TargetTile->GetTileIdentifier() : TEXT("???");
            FString UnitType = AIUnit->IsRangedAttack() ? TEXT("Sniper") : TEXT("Brawler");
            int32 Damage = FMath::RandRange(AIUnit->MinDamage, AIUnit->MaxDamage);
            GameMode->AddMoveToHistory(FString::Printf(TEXT("AI: %s attacks %s damage %d"), *UnitType, *TileName, Damage));
            
            TurnManager->RegisterAIAttack(AIUnit);
            return true;
        }
    }
    return false;
}

AUnitBase* ABattleManager::FindNearestEnemy(AUnitBase* AIUnit)
{
    if (!AIUnit)
    {
        UE_LOG(LogTemp, Error, TEXT("FindNearestEnemy: Unità AI non valida!"));
        return nullptr;
    }

    AUnitBase* NearestEnemy = nullptr;
    float MinDistance = FLT_MAX;

    for (AUnitBase* PlayerUnit : GameMode->PlayerUnits)
    {
        if (PlayerUnit && PlayerUnit->IsPlayerControlled()) 
        {
            float Distance = FVector::Dist2D(AIUnit->GetActorLocation(), PlayerUnit->GetActorLocation());
            if (Distance < MinDistance)
            {
                MinDistance = Distance;
                NearestEnemy = PlayerUnit;
            }
        }
    }

    if (NearestEnemy)
    {
        UE_LOG(LogTemp, Warning, TEXT("AI ha trovato il nemico più vicino: %s"), *NearestEnemy->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Nessun nemico trovato per AI."));
    }

    return NearestEnemy;
}