#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tile.h"
#include "PAASchifanoFrancesco/Units/UnitBase.h"
#include "GridManager.generated.h"

class UTurnManager;  // Forward declaration

class AUnitBase;

UCLASS()
class PAASCHIFANOFRANCESCO_API AGridManager : public AActor
{
	GENERATED_BODY()

public:
	AGridManager();
	virtual void Tick(float DeltaTime) override;
	void GenerateGrid();
	void GenerateObstacles();
	
	const TArray<ATile*>& GetGridTiles() const { return Grid; }
	
	TArray<ATile*> GetValidMovementTiles(AUnitBase* SelectedUnit);
	void HighlightMovementTiles(AUnitBase* SelectedUnit);
	void ClearHighlights();
	ATile* FindTileAtLocation(FVector Location);
	
	TArray<ATile*> GetValidAttackTiles(AUnitBase* Attacker);
	
	void HighlightTileUnderUnit(AUnitBase* Unit, const FLinearColor& Color);
	
	UPROPERTY()
	TArray<ATile*> HighlightedTiles;

	void OnUnitClicked(AUnitBase* TargetUnit);
	void HideAttackGrid();

	void FinalizeUnitMovement(AUnitBase* Unit, ATile* DestinationTile);
	
	void HighlightAttackGrid(AUnitBase* AttackingUnit);
	AUnitBase* GetUnitOnTile(ATile* Tile) const;


	UFUNCTION()
	TArray<ATile*> GetPathToTile(AUnitBase* Unit, ATile* Destination);

	UPROPERTY()
	class AMyGameMode* GameMode;
protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, Category = "Grid")
	int DimGridX = 25;

	UPROPERTY(EditAnywhere, Category = "Grid")
	int DimGridY = 25;

	UPROPERTY(EditAnywhere, Category = "Grid")
	float CellSize = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Grid")
	float Spacing = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Grid")
	float ObstaclePercentage = 0.3f;

	UPROPERTY(EditAnywhere, Category = "Grid")
	TSubclassOf<ATile> TileClass;

	TArray<ATile*> Grid;

	void DFS(ATile* CurrentTile, TSet<ATile*>& Visited, int32 MaxObstacles);
	TArray<ATile*> GetNeighbors(ATile* Tile);

	UPROPERTY()
	ATile* TileUnderSelectedUnit = nullptr;

	AUnitBase* CurrentSelectedUnit;
	UPROPERTY()
	UTurnManager* TurnManager;
	bool bAttackGridVisible = false;

	UPROPERTY()
	TArray<ATile*> AttackGridTiles;
};