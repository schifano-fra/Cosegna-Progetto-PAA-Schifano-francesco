// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tile.generated.h"

UCLASS()
class PAASCHIFANOFRANCESCO_API ATile : public AActor
{
	GENERATED_BODY()

public:
	// Constructor
	ATile();

	UFUNCTION()
	bool IsObstacle() const { return bIsObstacle; }
    
	UFUNCTION()
	void SetAsObstacle(bool NewbIsObstacle);
    
	UFUNCTION()
	bool GetHasPawn() const { return bHasPawn; }
    
	UFUNCTION()
	void SetHasPawn(bool Value) { bHasPawn = Value; }
    
	UFUNCTION()
	FVector GetPawnSpawnLocation() const;
	void SetTileIdentifier(const FString& NewIdentifier);

	void SetHighlight(bool bHighlight, FLinearColor HighlightColor);
	
	UFUNCTION(Category = "Tile")
	FString GetTileIdentifier() const { return TileIdentifier; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	// Mesh component for the tile
	UPROPERTY(VisibleAnywhere, Category = "Tile")
	UStaticMeshComponent* TileMesh;

	// Material for obstacle tiles
	UPROPERTY()
	UMaterialInstanceDynamic* TreeMaterial;

	// Material for obstacle tiles
	UPROPERTY()
	UMaterialInstanceDynamic* MountainMaterial;

	// Material for normal tiles
	UPROPERTY()
	UMaterialInstanceDynamic* NormalMaterial;

	bool bIsObstacle;
	bool bHasPawn;
	FString TileIdentifier;
	
};