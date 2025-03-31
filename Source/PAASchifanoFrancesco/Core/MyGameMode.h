// Creato da: Schifano Francesco, 5469994

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

// Includi i widget usati nel gioco
#include "PAASchifanoFrancesco/UI/UIMainMenu.h"
#include "PAASchifanoFrancesco/UI/UICOinFlip.h"
#include "PAASchifanoFrancesco/UI/StatusGameWidget.h"
#include "PAASchifanoFrancesco/UI/UITurnIndicator.h"
#include "PAASchifanoFrancesco/UI/InfoWidget.h"

// Includi il gestore della griglia
#include "PAASchifanoFrancesco/Grid/GridManager.h"

#include "MyGameMode.generated.h"

// Forward declarations per classi UI
class UUserWidget;
class USelectPawn;
class ABattleManager;
class APlacementManager;
class UTurnManager;
class AUnitMovementManager;

// Enum che rappresenta le fasi del gioco
UENUM()
enum class EGamePhase : uint8
{
	EInitial,     // Fase iniziale
	ECoinFlip,    // Lancio della moneta
	EPlacement,   // Fase di piazzamento delle unità
	EBattle,      // Fase di battaglia
	EGameOver     // Fine gioco
};

// Enum per rappresentare i due giocatori
UENUM()
enum class EPlayer : uint8
{
	Player1,  // Giocatore controllato dall'utente
	AI        // Intelligenza Artificiale
};

// Enum per impostare la difficoltà dell'AI
UENUM()
enum class EAILevel : uint8
{
	Easy,  // AI più semplice (movimenti casuali)
	Hard   // AI più complessa (scelte tattiche)
};

// Delegato per notificare il cambio di fase di gioco
DECLARE_MULTICAST_DELEGATE_OneParam(FOnGamePhaseChanged, EGamePhase);

// Classe principale del GameMode
UCLASS()
class PAASCHIFANOFRANCESCO_API AMyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	// Costruttore
	AMyGameMode();

	// Cambia la fase di gioco
	UFUNCTION()
	void SetGamePhase(EGamePhase NewPhase);

	// Ritorna la fase corrente
	UFUNCTION()
	EGamePhase GetCurrentGamePhase() const { return CurrentGamePhase; }

	// Delegato notificato quando cambia la fase di gioco
	FOnGamePhaseChanged OnGamePhaseChanged;
	
	virtual void BeginPlay() override;

	// Gestione delle varie fasi del gioco
	void HandleInitialPhase();
	void HandlePlacementPhase();
	void HandleBattlePhase();
	void HandleGameOver(const FString& WinnerName);

	// Gestione lancio della moneta e accesso risultato
	void FlipCoin();
	EPlayer GetCoinFlipResult() const { return StartingPlayer; }

	// Metodi di accesso ai manager del gioco
	APlacementManager* GetPlacementManager() const { return PlacementManager; }
	UTurnManager* GetTurnManager() const { return TurnManager; }
	ABattleManager* GetBattleManager() const { return BattleManager; }
	UStatusGameWidget* GetStatusGameWidget() const { return StatusGameWidget; }
	AGridManager* GetGridManager() const { return GridManager; }
	void SetGridManager(AGridManager* NewGridManager) { GridManager = NewGridManager; }
	UUITurnIndicator* GetTurnIndicatorWidget() const { return TurnIndicatorWidget; }

	// Liste delle unità controllate dal player e dall'AI
	TArray<AUnitBase*> PlayerUnits;
	TArray<AUnitBase*> AIUnits;

	// Aggiunge una riga allo storico delle mosse
	void AddMoveToHistory(const FString& Entry);

	// Riferimenti ai manager principali (Movement, Placement, Turni, Battaglia)
	UPROPERTY()
	APlacementManager* PlacementManager;

	UPROPERTY()
	UTurnManager* TurnManager;

	UPROPERTY()
	ABattleManager* BattleManager;

	UPROPERTY()
	AUnitMovementManager* GlobalMovementManager;

	// Widget attivi durante il gioco (status, indicatori, info)
	UPROPERTY()
	UStatusGameWidget* StatusGameWidget;

	UPROPERTY()
	UUITurnIndicator* TurnIndicatorWidget;

	UPROPERTY()
	UInfoWidget* InfoWidget;

	// Livello di difficoltà selezionato per l'AI
	UPROPERTY(EditAnywhere, Category = "AI")
	EAILevel AILevel = EAILevel::Hard;

protected:
	// Classi dei vari widget utilizzati nel gioco
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUIMainMenu> MainMenuClass; // Classe del menu principale

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<USelectPawn> SelectPawnClass; // Classe per selezione delle pedine

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUICOinFlip> CoinFlipWidgetClass; // Classe per il lancio della moneta

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUITurnIndicator> TurnIndicatorWidgetClass; // Classe per il widget che mostra il turno

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UStatusGameWidget> StatusGameWidgetClass; // Classe per il widget dello stato di gioco

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UGameOverWidget> GameOverWidgetClass; // Classe per il widget di fine gioco

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UInfoWidget> InfoWidgetClass; // Classe per il widget informativo

	// Riferimenti ai widget attivi istanziati
	UPROPERTY()
	AGridManager* GridManager; // Manager della griglia

	UPROPERTY()
	USelectPawn* SelectPawn; // Widget per selezione delle pedine

	UPROPERTY()
	UUserWidget* CoinFlipWidget; // Widget attivo per lancio moneta

	UPROPERTY()
	UGameOverWidget* GameOverWidget; // Widget attivo per game over

private:
	FString Winner; // Nome del vincitore da mostrare nel widget
	EGamePhase CurrentGamePhase; // Fase corrente del gioco
	EPlayer StartingPlayer; // Giocatore che inizia (dopo coin flip)
	FTimerHandle AITurnDelayHandle; // Timer per il delay tra azioni AI
};