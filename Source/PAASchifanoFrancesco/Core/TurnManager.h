#pragma once

#include "CoreMinimal.h"
#include "MyGameMode.h"
#include "TurnManager.generated.h"

class ABattleManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCanEndTurn, bool, IsVisible);

UCLASS()
class PAASCHIFANOFRANCESCO_API UTurnManager : public UObject
{
    GENERATED_BODY()

public:
    void Initialize(AMyGameMode* InGameMode, ABattleManager* InBattleManager = nullptr);
    void StartTurn();
    void EndTurn();
    
    void RegisterPlayerMove(AUnitBase* Unit);
    void RegisterPlayerAttack(AUnitBase* Unit);
    void RegisterAIMove(AUnitBase* Unit);
    void RegisterAIAttack(AUnitBase* Unit);
    
    void RegisterPlacementMove(AUnitBase* Unit);
    void UpdateTurnUI();
    void CheckPlayerEndTurn(AUnitBase* Unit);
    void SetInitialPlayer(EPlayer StartingPlayer);
    
    EPlayer GetCurrentPlayer() const { return CurrentPlayer; }
private:
    UPROPERTY()
    AMyGameMode* GameMode;

    UPROPERTY()
    ABattleManager* BattleManager;

    UPROPERTY()
    EPlayer CurrentPlayer;

    UPROPERTY()
    FOnCanEndTurn OnCanEndTurn;
    
    FTimerHandle AITurnTimerHandle;
};