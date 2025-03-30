// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PAASchifanoFrancesco/Units/UnitMovementManager.h"
#include "GameFramework/Actor.h"
#include "TurnManager.h"
#include "BattleManager.generated.h"

class AUnitBase;

UCLASS()
class PAASCHIFANOFRANCESCO_API ABattleManager : public AActor
{
	GENERATED_BODY()

public:
	ABattleManager();

	void StartBattle();  // Avvia la fase di battaglia
	void OnAIMovementComplete(AUnitBase* UnitBase);
	
	void PrepareAITurn();
	bool TryAIAttack(AUnitBase* AIUnit);
	void TryAIMove(AUnitBase* AIUnit);
	void TryAIRandomMove(AUnitBase* AIUnit);

	AUnitBase* FindNearestEnemy(AUnitBase* AIUnit);

private:
	AUnitBase* SelectedUnit;  // L'unità attualmente selezionata
	class AGridManager* GridManager;  // Riferimento al GridManager
	class AMyGameMode* GameMode;  // Riferimento al GameMode

	int32 CurrentAIIndex;
	TArray<AUnitBase*> AIUnitsToProcess;
    
	void ProcessNextAIUnit();
	
	UPROPERTY()
	UTurnManager* TurnManager;
	
	UPROPERTY()
	AUnitMovementManager* MovementManager;
};