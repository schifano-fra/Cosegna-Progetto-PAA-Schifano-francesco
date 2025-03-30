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

AMyPlayerController::AMyPlayerController()
{
    SelectedUnit = nullptr;
    //bIsAttackGridVisible = false;
    bIsGridLocked = false;
    MovementTileColor = FLinearColor(0.0f, 0.5f, 1.0f);
}

void AMyPlayerController::BeginPlay()
{
    Super::BeginPlay();

    GameMode = Cast<AMyGameMode>(UGameplayStatics::GetGameMode(this));
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;

    if (GameMode && GameMode->GlobalMovementManager)
    {
        GridManager = GameMode->GetGridManager();
        MovementManager = GameMode->GlobalMovementManager;
        MovementManager->OnMovementFinished.AddDynamic(this, &AMyPlayerController::OnUnitMovementFinished);
    }
    else
    {
        UE_LOG(LogTemp, Fatal, TEXT("❌ MovementManager o GameMode mancante."));
    }
}

void AMyPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (!InputComponent) return;

    InputComponent->BindAction("LeftClick", IE_Pressed, this, &AMyPlayerController::OnLeftClick);
    InputComponent->BindAction("RightClick", IE_Pressed, this, &AMyPlayerController::OnRightClick);
}

void AMyPlayerController::OnLeftClick()
{
    FInputModeGameOnly InputMode;
    InputMode.SetConsumeCaptureMouseDown(false); // Evita che UI blocchi click
    SetInputMode(InputMode);
   
    if (bIsGridLocked || !GameMode || !GameMode->TurnManager) return;

    if (!GridManager) GridManager = GameMode->GetGridManager();
    if (!GridManager) return;

    FHitResult Hit;
    if (!GetHitResultUnderCursor(ECC_Visibility, false, Hit) || !Hit.GetActor()) return;

    AActor* HitActor = Hit.GetActor();

    switch (GameMode->GetCurrentGamePhase())
    {
        case EGamePhase::EPlacement:
            HandlePlacementClick(HitActor);
            break;
        case EGamePhase::EBattle:
            HandleBattleClick(HitActor, true);
            break;
        default:
            break;
    }
}

void AMyPlayerController::HandlePlacementClick(AActor* HitActor)
{
    if (GameMode->TurnManager->GetCurrentPlayer() != EPlayer::Player1) return;

    APlacementManager* PlacementManager = GameMode->GetPlacementManager();
    if (!PlacementManager) return;

    if (ATile* ClickedTile = Cast<ATile>(HitActor))
    {
        PlacementManager->HandleTileClick(ClickedTile);
    }
}

void AMyPlayerController::HandleBattleClick(AActor* HitActor, bool isLeft)
{
    if (!GameMode || !GameMode->TurnManager || GameMode->GetCurrentGamePhase() != EGamePhase::EBattle) return;
    if (GameMode->TurnManager->GetCurrentPlayer() != EPlayer::Player1 || bIsGridLocked) return;

    // Clic su una TILE (potrebbe essere movimento)
    if (ATile* ClickedTile = Cast<ATile>(HitActor))
    {
        if (isLeft && SelectedUnit)
        {
            const TArray<ATile*> ValidTiles = GridManager->GetValidMovementTiles(SelectedUnit);
            if (ValidTiles.Contains(ClickedTile) && SelectedUnit->GetCurrentAction() == EUnitAction::Idle)
            {
                TryMoveToTile(ClickedTile);
            }
            else
            {
                // Click su tile non valida o movimento non possibile → annulla selezione
                GridManager->ClearHighlights();
                SelectedUnit = nullptr;
            }
        }
        return;
    }

    // Clic su UNITÀ
    else if (AUnitBase* ClickedUnit = Cast<AUnitBase>(HitActor))
    {
        // Se è un'unità del player
        if (ClickedUnit->IsPlayerControlled())
        {
            // 🔁 Clic sulla STESSA unità selezionata
            if (ClickedUnit == SelectedUnit)
            {
                if (isLeft)
                {
                    GridManager->ClearHighlights();
                    SelectedUnit = nullptr;
                }
                else if (SelectedUnit->CanAct())
                {
                    GridManager->ClearHighlights();
                    GridManager->HighlightTileUnderUnit(SelectedUnit, FLinearColor(0.8f, 0.4f, 0.0f));
                    GridManager->HighlightAttackGrid(SelectedUnit);
                }
            }
            else
            {
                // 🔄 Cambio selezione a un'altra unità player
                GridManager->ClearHighlights();
                SelectedUnit = ClickedUnit;

                GridManager->HighlightTileUnderUnit(SelectedUnit, FLinearColor(0.8f, 0.4f, 0.0f));
                if (SelectedUnit->CanAct())
                {
                    if (isLeft && SelectedUnit->GetCurrentAction() == EUnitAction::Idle)
                    {
                        GridManager->HighlightMovementTiles(SelectedUnit);
                    }
                    else if (!isLeft)
                    {
                        GridManager->HighlightAttackGrid(SelectedUnit);
                    }
                }
            }
        }

        // Clic su unità NEMICA
        else
        {
            if (!isLeft && SelectedUnit && SelectedUnit->CanAct())
            {
                TArray<ATile*> AttackTiles = GridManager->GetValidAttackTiles(SelectedUnit);
                ATile* TargetTile = GridManager->FindTileAtLocation(ClickedUnit->GetActorLocation());

                if (AttackTiles.Contains(TargetTile))
                {
                    GridManager->ClearHighlights();
                    TryAttack(SelectedUnit, ClickedUnit);
                }
            }
        }
    }
}

void AMyPlayerController::TryAttack(AUnitBase* Attacker, AUnitBase* Defender)
{
    if (!Attacker || Attacker->GetCurrentAction() == EUnitAction::Attacked || Attacker->GetCurrentAction() == EUnitAction::MoveAttack)
    {
        UE_LOG(LogTemp, Warning, TEXT("❌ %s non può attaccare: ha già attaccato o fatto move+attack."), *Attacker->GetName());
        return;
    }
    ATile* TileDefender = GridManager->FindTileAtLocation(Defender->GetActorLocation());
    TArray<ATile*> AttackArea = GridManager->GetValidAttackTiles(Attacker);
    if (AttackArea.Contains(TileDefender))
    {
        ExecuteAttack(Attacker, Defender);
    }
}

void AMyPlayerController::TryMoveToTile(ATile* ClickedTile)
{
    // Evita selezione o movimento se l’unità non è idle
    if (!SelectedUnit || !SelectedUnit->CanAct()) return;
    const TArray<ATile*> ValidTiles = GridManager->GetValidMovementTiles(SelectedUnit);
    if (!ValidTiles.Contains(ClickedTile) || SelectedUnit->bHasMovedThisTurn) return;

    const TArray<ATile*> Path = GridManager->GetPathToTile(SelectedUnit, ClickedTile);
    if (Path.Num() > SelectedUnit->GetMovementRange())
    {
        UE_LOG(LogTemp, Warning, TEXT("❌ Percorso troppo lungo."));
        GridManager->ClearHighlights();
        return;
    }

    GridManager->ClearHighlights();
    SelectedUnit->SetCurrentAction(EUnitAction::Moved);
    SetMovementLocked(true);
    MovementManager->MoveUnit(SelectedUnit, Path, 300.f);

    ATile* From = GridManager->FindTileAtLocation(SelectedUnit->GetActorLocation());
    FString FromName = From ? From->GetTileIdentifier() : TEXT("???");
    FString ToName = ClickedTile->GetTileIdentifier();
    FString UnitType = SelectedUnit->IsRangedAttack() ? TEXT("Sniper") : TEXT("Brawler");
    GameMode->AddMoveToHistory(FString::Printf(TEXT("Player: %s moves from %s to %s"), *UnitType, *FromName, *ToName));
}

void AMyPlayerController::OnRightClick()
{
    FInputModeGameOnly InputMode;
    InputMode.SetConsumeCaptureMouseDown(false); // Evita che UI blocchi click
    SetInputMode(InputMode);
    if (GameMode->GetCurrentGamePhase()==EGamePhase::EGameOver)
    {
        GameMode->HandleGameOver(GameMode->PlayerUnits.Num() == 0 ? TEXT("AI") : TEXT("Player"));
    }
    if (bIsGridLocked || !GameMode || !GameMode->TurnManager) return;

    if (!GridManager) GridManager = GameMode->GetGridManager();
    if (!GridManager) return;

    FHitResult Hit;
    if (!GetHitResultUnderCursor(ECC_Visibility, false, Hit) || !Hit.GetActor()) return;

    AActor* HitActor = Hit.GetActor();

    switch (GameMode->GetCurrentGamePhase())
    {
    case EGamePhase::EPlacement:
        HandlePlacementClick(HitActor);
        break;
    case EGamePhase::EBattle:
        HandleBattleClick(HitActor, false);
        break;
    default:
        break;
    }
}

void AMyPlayerController::ExecuteAttack(AUnitBase* Attacker, AUnitBase* Defender)
{
    if (GameMode && GameMode->GetCurrentGamePhase() == EGamePhase::EGameOver)
    {
        UE_LOG(LogTemp, Warning, TEXT("⛔ Turno IA interrotto: gioco terminato."));
        return;
    }
    if (!Attacker || !Defender) return;
    UStatusGameWidget* StatusGame = nullptr;
    if (GameMode)
    {
        StatusGame = GameMode->GetStatusGameWidget();
    }

    SetMovementLocked(true);

    Attacker->AttackUnit(Defender);
    
    ATile* DefenderTile = GridManager->FindTileAtLocation(Defender->GetActorLocation());
    FString TileName = DefenderTile ? DefenderTile->GetTileIdentifier() : TEXT("???");
    FString UnitType = Attacker->IsRangedAttack() ? TEXT("Sniper") : TEXT("Brawler");
    int32 Damage = FMath::RandRange(Attacker->MinDamage, Attacker->MaxDamage); // serve per la UI
    GameMode->AddMoveToHistory(FString::Printf(TEXT("Player: %s attacks %s damage %d"), *UnitType, *TileName, Damage));

    Attacker->SetCurrentAction(EUnitAction::Attacked);

    UE_LOG(LogTemp, Warning, TEXT("🎯 %s ha attaccato %s"), *Attacker->GetName(), *Defender->GetName());

    // Contrattacco: solo se l'attaccante è uno Sniper
    if (Attacker->IsRangedAttack())//se è true vuol dire che è sniper
    {
        const bool DefenderIsSniper = Defender->IsRangedAttack();
        const bool DefenderIsBrawlerClose =
            !Defender->IsRangedAttack() &&
            FVector::Dist2D(Attacker->GetActorLocation(), Defender->GetActorLocation()) <= 110.f;

        if (DefenderIsSniper || DefenderIsBrawlerClose)
        {
            int32 CounterDamage = FMath::RandRange(1, 3);
            UE_LOG(LogTemp, Warning, TEXT("🔁 Contrattacco! %s infligge %d danni a %s"), *Defender->GetName(), CounterDamage, *Attacker->GetName());

            FPointDamageEvent DamageEvent;
            Attacker->TakeDamage(CounterDamage, DamageEvent, nullptr, Defender);

            ATile* AttackerTile = GridManager->FindTileAtLocation(Attacker->GetActorLocation());
            TileName = AttackerTile ? AttackerTile->GetTileIdentifier() : TEXT("???");
            FString DefenderType = Defender->IsRangedAttack() ? TEXT("Sniper") : TEXT("Brawler");
            GameMode->AddMoveToHistory(FString::Printf(TEXT("AI: %s attacks %s damage %d"), *DefenderType, *TileName, CounterDamage));
        }
        if (Attacker->IsDead())
        {
            Attacker->Die(Attacker);
        }
        else
        {
            StatusGame->UpdateUnitHealth(Attacker, Attacker->GetHealthPercent());
        }
    }

    GridManager->ClearHighlights();
    SelectedUnit = nullptr;
    // ❗ Delegate: possiamo avvisare il TurnManager qui
    if (GameMode && GameMode->TurnManager)
    {
        GameMode->TurnManager->RegisterPlayerAttack(Attacker);
    }
    SetMovementLocked(false);
}

void AMyPlayerController::OnUnitMovementFinished(AUnitBase* Unit)
{
    SelectedUnit = nullptr;

    if (GameMode && GameMode->TurnManager)
    {
        GameMode->TurnManager->RegisterPlayerMove(Unit);
    }
    SetMovementLocked(false);
}

void AMyPlayerController::SetMovementLocked(bool bLocked)
{
    bIsGridLocked = bLocked;
    UE_LOG(LogTemp, Warning, TEXT("%s input player"), bLocked ? TEXT("🔒 Blocco") : TEXT("🔓 Sblocco"));
}