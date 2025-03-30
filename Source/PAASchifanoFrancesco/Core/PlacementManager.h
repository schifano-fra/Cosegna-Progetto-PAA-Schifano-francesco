// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyGameMode.h"
#include "GameFramework/Actor.h"
#include "PlacementManager.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnPlacementCompleted);

UCLASS()
class PAASCHIFANOFRANCESCO_API APlacementManager : public AActor
{
	GENERATED_BODY()

public:
	APlacementManager();

	void Initialize(TSubclassOf<USelectPawn> InSelectPawnClass);
	void StartPlacement();
	void SetSelectedPawnType(TSubclassOf<AUnitBase> PawnType, EPlayer Player);
	void HandleTileClick(ATile* ClickedTile);
	void FinishPlacementPhase();
	void PlaceAIPawn();

	FOnPlacementCompleted OnPlacementCompleted;

private:
	AMyGameMode* GM;
	AGridManager* GridManager;
	USelectPawn* SelectPawn;

	TSubclassOf<AUnitBase> PlayerPawnType;
	TSubclassOf<AUnitBase> AIPawnType;
	
	UPROPERTY()
	TSubclassOf<USelectPawn> SelectPawnClass;
	
	void PlacePlayerPawn(ATile* ClickedTile);
	void CheckPlacementCompletion();
};
