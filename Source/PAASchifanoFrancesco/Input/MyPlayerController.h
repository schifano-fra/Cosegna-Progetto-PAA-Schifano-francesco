// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PAASchifanoFrancesco/Units/UnitMovementManager.h"
#include "PAASchifanoFrancesco/Units/UnitBase.h"
#include "PAASchifanoFrancesco/Grid/GridManager.h"
#include "MyPlayerController.generated.h"

UCLASS()
class PAASCHIFANOFRANCESCO_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMyPlayerController();
	UFUNCTION()
	void SetMovementLocked(bool bLocked);

	UFUNCTION()
	void OnMovementDone()
	{
		UE_LOG(LogTemp, Warning, TEXT("🔓 Sblocco input player da OnMovementDone"));
		SetMovementLocked(false);
	}

	UFUNCTION()
	void OnUnitMovementFinished(AUnitBase* Unit);
	
protected:
	virtual void BeginPlay() override;
	
	virtual void SetupInputComponent() override;

private:
	class AMyGameMode* GameMode;  // Riferimento al GameMode
	// Input Handlers
	void OnLeftClick();
	void HandlePlacementClick(AActor* HitActor);
	void HandleBattleClick(AActor* HitActor, bool isLeft);
	void TryAttack(AUnitBase* Attacker, AUnitBase* Defender);
	void TryMoveToTile(ATile* ClickedTile);
	//void ToggleMovementGrid(AUnitBase* Unit);
	void OnRightClick();
	void ExecuteAttack(AUnitBase* Attacker, AUnitBase* Defender);

	// Selected Unit
	AUnitBase* SelectedUnit;

	// Visibility States
	//bool bIsMovementGridVisible = false;
	//bool bIsAttackGridVisible;
	bool bIsGridLocked = false;

	// Colors for Highlighting
	FLinearColor MovementTileColor;
	UPROPERTY()
	AUnitMovementManager* MovementManager;
	AGridManager* GridManager;

	FTimerHandle GridHideTimerHandle;
};