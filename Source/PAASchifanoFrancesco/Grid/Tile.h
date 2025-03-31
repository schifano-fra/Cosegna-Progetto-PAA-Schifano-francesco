// Creato da: Schifano Francesco 5469994

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tile.generated.h"

/**
 * Descrizione:
 * Rappresenta una singola cella della griglia nel gioco. Ogni tile può essere
 * un ostacolo, contenere un'unità, oppure essere selezionata e colorata durante
 * la fase di movimento o attacco. Gestisce inoltre i materiali associati (normale,
 * albero, montagna) e offre funzioni per identificarla.
 */
UCLASS()
class PAASCHIFANOFRANCESCO_API ATile : public AActor
{
	GENERATED_BODY()

public:
	/** Costruttore */
	ATile();

	/** Ritorna true se questa tile è un ostacolo */
	UFUNCTION()
	bool IsObstacle() const { return bIsObstacle; }

	/** Imposta la tile come ostacolo e aggiorna il materiale visivo */
	UFUNCTION()
	void SetAsObstacle(bool NewbIsObstacle);

	/** Ritorna true se c'è un'unità sopra la tile */
	UFUNCTION()
	bool GetHasPawn() const { return bHasPawn; }

	/** Imposta se la tile ha o meno un'unità */
	UFUNCTION()
	void SetHasPawn(bool Value) { bHasPawn = Value; }

	/** Ritorna la posizione 3D dove far spawnare una pedina sopra la tile */
	UFUNCTION()
	FVector GetPawnSpawnLocation() const;

	/** Imposta l'identificativo testuale di questa tile */
	void SetTileIdentifier(const FString& NewIdentifier);

	/** Evidenzia la tile con un colore specificato */
	void SetHighlight(bool bHighlight, FLinearColor HighlightColor);

	/** Restituisce l'identificatore della tile (es. A1, B2, ecc.) */
	UFUNCTION(Category = "Tile")
	FString GetTileIdentifier() const { return TileIdentifier; }

protected:
	/** Chiamato quando il gioco inizia o quando l’attore viene spawnato */
	virtual void BeginPlay() override;

public:
	/** Tick dell’attore. Attualmente non utilizzato. */
	virtual void Tick(float DeltaTime) override;

private:
	/** Mesh visiva della tile */
	UPROPERTY(VisibleAnywhere, Category = "Tile")
	UStaticMeshComponent* TileMesh;

	/** Materiale dinamico per ostacolo tipo albero */
	UPROPERTY()
	UMaterialInstanceDynamic* TreeMaterial;

	/** Materiale dinamico per ostacolo tipo montagna */
	UPROPERTY()
	UMaterialInstanceDynamic* MountainMaterial;

	/** Materiale dinamico per tile normale */
	UPROPERTY()
	UMaterialInstanceDynamic* NormalMaterial;

	/** Flag: indica se la tile è un ostacolo */
	bool bIsObstacle;

	/** Flag: indica se la tile è occupata da un'unità */
	bool bHasPawn;

	/** Identificatore della tile (es. A1, B3...) */
	FString TileIdentifier;
};