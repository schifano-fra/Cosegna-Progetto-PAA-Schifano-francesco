// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PAASchifanoFrancesco/UI/UIMainMenu.h"
#include "PAASchifanoFrancesco/UI/UICOinFlip.h"
#include "PAASchifanoFrancesco/UI/StatusGameWidget.h"
#include "PAASchifanoFrancesco/UI/UITurnIndicator.h"
#include "PAASchifanoFrancesco/UI/InfoWidget.h"
#include "PAASchifanoFrancesco/Grid/GridManager.h"
#include "MyGameMode.generated.h"

class UUserWidget;
class USelectPawn;

UENUM()
enum class EGamePhase : uint8
{
	EInitial,
	ECoinFlip,
	EPlacement,
	EBattle,
	EGameOver
};

UENUM()
enum class EPlayer : uint8
{
	Player1,
	AI
};

UENUM()
enum class EAILevel : uint8
{
	Easy,
	Hard
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnGamePhaseChanged, EGamePhase);

UCLASS()
class PAASCHIFANOFRANCESCO_API AMyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMyGameMode();

	// Cambia la fase di gioco
	UFUNCTION()
	void SetGamePhase(EGamePhase NewPhase);

	// Ottieni la fase corrente
	UFUNCTION()
	EGamePhase GetCurrentGamePhase() const { return CurrentGamePhase; }

	// Delegato per notificare il cambio di fase
	FOnGamePhaseChanged OnGamePhaseChanged;

	virtual void BeginPlay() override;
	void HandleInitialPhase();

	UPROPERTY()
	class APlacementManager* PlacementManager;
	void HandlePlacementPhase();
	void InitGridManager();
	APlacementManager* GetPlacementManager() const;
	
	void HandleBattlePhase();
	void HandleGameOver(const FString& WinnerName);

	UUITurnIndicator* GetTurnIndicatorWidget() const { return TurnIndicatorWidget; }
	UPROPERTY()
	class UTurnManager* TurnManager;

	UPROPERTY()
	class ABattleManager* BattleManager;
	
	UFUNCTION(BlueprintCallable)
	void FlipCoin();  // Metodo per il lancio della moneta

	UFUNCTION(BlueprintPure)
	EPlayer GetCoinFlipResult() const { return StartingPlayer; }

	UTurnManager* GetTurnManager() const { return TurnManager; }
	
	ABattleManager* GetBattleManager() const { return BattleManager; }

	UStatusGameWidget* GetStatusGameWidget() const { return StatusGameWidget; }
	
	AGridManager* GetGridManager() const { return GridManager; }
	void SetGridManager(AGridManager* NewGridManager) { GridManager = NewGridManager; }

	TArray<AUnitBase*> PlayerUnits;
	TArray<AUnitBase*> AIUnits;

	UPROPERTY()
	class AUnitMovementManager* GlobalMovementManager;

	UPROPERTY()
	UStatusGameWidget* StatusGameWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI")
	EAILevel AILevel = EAILevel::Hard;
	
	void AddMoveToHistory(const FString& Entry);

	
protected:
	// Variabili per i widget (Main Menu e Select Pawn)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUIMainMenu> MainMenuClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<USelectPawn> SelectPawnClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUICOinFlip> CoinFlipWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUITurnIndicator> TurnIndicatorWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UStatusGameWidget> StatusGameWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UGameOverWidget> GameOverWidgetClass;

	UPROPERTY()
	UInfoWidget* InfoWidget;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UInfoWidget> InfoWidgetClass;
	
	UUserWidget* CoinFlipWidget;

	UUITurnIndicator* TurnIndicatorWidget;
	
	// Riferimenti interni
	UPROPERTY()
	AGridManager* GridManager;

	UPROPERTY()
	USelectPawn* SelectPawn;

	UGameOverWidget* GameOverWidget;

private:
	FString Winner;
	EGamePhase CurrentGamePhase;
	EPlayer StartingPlayer;
	FTimerHandle AITurnDelayHandle;
};