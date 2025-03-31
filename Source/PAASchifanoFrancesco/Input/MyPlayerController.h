// Creato da: Schifano Francesco 5469994

#pragma once

// Creato da: Schifano Francesco 5469994

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PAASchifanoFrancesco/Units/UnitMovementManager.h"
#include "PAASchifanoFrancesco/Units/UnitBase.h"
#include "PAASchifanoFrancesco/Grid/GridManager.h"
#include "MyPlayerController.generated.h"

class AMyGameMode;

/**
 * Classe: AMyPlayerController
 * 
 * Descrizione:
 * Controller personalizzato del giocatore. Intercetta input da mouse e gestisce le azioni 
 * relative alla selezione, movimento e attacco delle unità. Coordina le interazioni con 
 * la griglia e l'interfaccia durante le fasi di piazzamento e battaglia.
 */
UCLASS()
class PAASCHIFANOFRANCESCO_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	/** Costruttore del controller */
	AMyPlayerController();

	/**
	 * Metodo: SetMovementLocked
	 * 
	 * Imposta il flag per bloccare o sbloccare temporaneamente i click sulla griglia.
	 * Utile per impedire input multipli durante il movimento o l'attacco.
	 */
	UFUNCTION()
	void SetMovementLocked(bool bLocked);

	/**
	 * Metodo: OnUnitMovementFinished
	 * 
	 * Callback chiamata al termine di un movimento unità.
	 * Permette al controller di aggiornare lo stato interno o agire in base alla logica.
	 */
	UFUNCTION()
	void OnUnitMovementFinished(AUnitBase* Unit);

protected:

	/**
	 * Metodo: BeginPlay
	 * 
	 * Recupera riferimenti iniziali come GameMode e MovementManager.
	 */
	virtual void BeginPlay() override;

	/**
	 * Metodo: SetupInputComponent
	 * 
	 * Collega i click del mouse alle funzioni personalizzate `OnLeftClick` e `OnRightClick`.
	 */
	virtual void SetupInputComponent() override;

private:

	/** Riferimento al GameMode principale */
	AMyGameMode* GameMode;

	/** Funzione che gestisce il click sinistro del mouse */
	void OnLeftClick();

	/** Gestisce il click durante la fase di piazzamento */
	void HandlePlacementClick(AActor* HitActor);

	/** Gestisce il click durante la fase di battaglia (sia sinistro che destro) */
	void HandleBattleClick(AActor* HitActor, bool isLeft);

	/** Esegue i controlli e la logica per un attacco tra due unità */
	void TryAttack(AUnitBase* Attacker, AUnitBase* Defender);

	/** Prova a muovere l’unità selezionata sulla tile cliccata */
	void TryMoveToTile(ATile* ClickedTile);

	/** Funzione associata al click destro del mouse */
	void OnRightClick();

	/** Esegue un attacco confermato tra un attaccante e un difensore */
	void ExecuteAttack(AUnitBase* Attacker, AUnitBase* Defender);

	/** Riferimento all'unità attualmente selezionata dal giocatore */
	AUnitBase* SelectedUnit;

	/** Flag che blocca temporaneamente la griglia per evitare azioni multiple */
	bool bIsGridLocked = false;

	/** Riferimento al MovementManager globale (movimenti delle unità) */
	UPROPERTY()
	AUnitMovementManager* MovementManager;

	/** Riferimento al GridManager per mostrare griglie di movimento e attacco */
	AGridManager* GridManager;

	/** Timer usato per nascondere la griglia dopo un certo tempo */
	FTimerHandle GridHideTimerHandle;
};