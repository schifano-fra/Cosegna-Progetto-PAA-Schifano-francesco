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

void UTurnManager::Initialize(AMyGameMode* InGameMode, ABattleManager* InBattleManager)
{
    GameMode = InGameMode;
    BattleManager = InBattleManager;
    CurrentPlayer = EPlayer::Player1;
    if (GameMode)
    {
        UStatusGameWidget* StatusGameWidget = GameMode->GetStatusGameWidget();
        if (StatusGameWidget)
        {
            // 🔹 Collega il delegato OnCanEndTurn alla funzione ActiveButton del widget
            OnCanEndTurn.AddDynamic(StatusGameWidget, &UStatusGameWidget::ActiveButton);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("❌ ERRORE: StatusGameWidget non trovato in GameMode!"));
        }
    }
}

void UTurnManager::StartTurn()
{
    UpdateTurnUI();

    UE_LOG(LogTemp, Warning, TEXT("[TurnManager] Inizia il turno di: %s"), *UEnum::GetValueAsString(CurrentPlayer));

    if (GameMode->GetCurrentGamePhase() == EGamePhase::EPlacement)
    {
        if (CurrentPlayer == EPlayer::AI)
        {
            UE_LOG(LogTemp, Warning, TEXT("Turno dell'AI per piazzare un'unità."));

            if (APlacementManager* PM = GameMode->GetPlacementManager())
            {
                GameMode->GetWorld()->GetTimerManager().SetTimer(AITurnTimerHandle, [this, PM]()
                {
                    PM->PlaceAIPawn();
                }, 3.0f, false);
            }
        }
    }
    else if (GameMode->GetCurrentGamePhase() == EGamePhase::EBattle)
    {
        // 🔴 Nascondi sempre il pulsante di End Turn a inizio turno Player
        if (GameMode->GetStatusGameWidget())
        {
            GameMode->GetStatusGameWidget()->ActiveButton(false);  // Disabilita e nasconde
        }
        if (CurrentPlayer == EPlayer::Player1)
        {
            // 🔓 Sblocca l'input
            if (APlayerController* PC = UGameplayStatics::GetPlayerController(GameMode, 0))
            {
                if (AMyPlayerController* MyPC = Cast<AMyPlayerController>(PC))
                {
                    MyPC->SetMovementLocked(false);
                }
            }
        }
        else if (CurrentPlayer == EPlayer::AI && BattleManager)
        {
            UE_LOG(LogTemp, Warning, TEXT("🟢 L'AI inizia il turno e controlla se può attaccare"));

            // Prima di muoversi, l'AI verifica se può attaccare
            BattleManager->PrepareAITurn();
        }
    }
}

void UTurnManager::EndTurn()
{
    UE_LOG(LogTemp, Warning, TEXT("🔄 EndTurn() chiamato - Cambio turno in corso..."));

    AGridManager* GridManager = GameMode->GetGridManager();
    if (GridManager)
    {
        GridManager->ClearHighlights();
    }

    // Cambia turno e resetta i contatori
    if (CurrentPlayer == EPlayer::Player1)
    {
        for (AUnitBase* PlayerUnit : GameMode->PlayerUnits)
        {
            PlayerUnit->ResetAction();
        }
        UE_LOG(LogTemp, Warning, TEXT("🔄 Cambio turno: Ora è il turno dell'AI."));
        CurrentPlayer = EPlayer::AI;
    }
    else
    {
        for (AUnitBase* AIUnit : GameMode->AIUnits)
        {
            AIUnit->ResetAction();
        }
        UE_LOG(LogTemp, Warning, TEXT("🔄 Cambio turno: Ora è il turno del Player."));
        CurrentPlayer = EPlayer::Player1;
    }

    // Aspetta 1 secondo prima di avviare il nuovo turno
    GameMode->GetWorld()->GetTimerManager().SetTimer(AITurnTimerHandle, this, &UTurnManager::StartTurn, 1.0f, false);
}


void UTurnManager::RegisterPlayerMove(AUnitBase* Unit)
{
    UE_LOG(LogTemp, Error, TEXT("SONO IN RegisterPlayerMove(AUnitBase* Unit)"));
    Unit->SetCurrentAction(EUnitAction::Moved);
    CheckPlayerEndTurn(Unit);
}

void UTurnManager::RegisterPlayerAttack(AUnitBase* Unit)
{
    UE_LOG(LogTemp, Warning, TEXT("🗡️ RegisterPlayerAttack chiamato per %s"), *Unit->GetName());
    Unit->SetCurrentAction(EUnitAction::Attacked);
    CheckPlayerEndTurn(Unit);
}


void UTurnManager::RegisterAIMove(AUnitBase* Unit)
{
    Unit->SetCurrentAction(EUnitAction::Moved);
}

void UTurnManager::RegisterAIAttack(AUnitBase* Unit)
{
    UE_LOG(LogTemp, Warning, TEXT("🗡️ RegisterAIAttack chiamato per %s"), *Unit->GetName());
    Unit->SetCurrentAction(EUnitAction::Attacked);
}

void UTurnManager::RegisterPlacementMove(AUnitBase* Unit)
{
    if (CurrentPlayer == EPlayer::Player1)
    {
        GameMode->PlayerUnits.Add(Unit);
    }
    else
    {
        GameMode->AIUnits.Add(Unit);
    }

    if (GameMode->PlayerUnits.Num()>1 &&  GameMode->AIUnits.Num()>1)
    {
        if (GameMode && GameMode->GetPlacementManager())
        {
            GameMode->GetPlacementManager()->FinishPlacementPhase();
        }
    }
    else
    {
        CurrentPlayer = (CurrentPlayer == EPlayer::Player1) ? EPlayer::AI : EPlayer::Player1;
        StartTurn();
    }
}

void UTurnManager::SetInitialPlayer(EPlayer StartingPlayer)
{
    CurrentPlayer = StartingPlayer;
}

void UTurnManager::UpdateTurnUI()
{
    if (GameMode && GameMode->GetTurnIndicatorWidget())
    {
        FString PlayerName = (CurrentPlayer == EPlayer::Player1) ? TEXT("Player") : TEXT("AI");
        GameMode->GetTurnIndicatorWidget()->UpdateTurnText(PlayerName);
    }
}

void UTurnManager::CheckPlayerEndTurn(AUnitBase* Unit)
{
    if (!GameMode) return;
    if (!Unit || !Unit->IsPlayerControlled()) return;

    bool bHasMovedUnit = false;
    bool bHasIdleUnit = false;

    for (AUnitBase* PlayerUnit : GameMode->PlayerUnits)
    {
        EUnitAction Action = PlayerUnit->GetCurrentAction();

        if (Action == EUnitAction::Idle)
        {
            UE_LOG(LogTemp, Warning, TEXT("⏳ Unità %s è ancora in Idle. Il turno non può finire."), *PlayerUnit->GetName());
            bHasIdleUnit = true;
            break;  // Basta una Idle per fermare tutto
        }
        if (Action == EUnitAction::Moved)
        {
            bHasMovedUnit = true;
        }
    }

    if (bHasIdleUnit)
    {
        return; // Aspettiamo che tutte agiscano
    }

    if (bHasMovedUnit)
    {
        UE_LOG(LogTemp, Warning, TEXT("✅ Nessuna unità è Idle, e almeno una ha solo mosso. Mostro pulsante di fine turno."));
        OnCanEndTurn.Broadcast(true); // Appare il pulsante
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("✅ Nessuna unità è Idle o Moved. Tutte hanno attaccato. Fine turno automatica."));
        EndTurn(); // Nessuna Idle o Moved → tutte hanno attaccato → fine turno
    }
}