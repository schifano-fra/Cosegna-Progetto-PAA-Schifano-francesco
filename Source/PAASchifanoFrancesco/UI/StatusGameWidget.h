// Creato da: Schifano Francesco, 5469994

#pragma once

// Include fondamentale di Unreal Engine
#include "CoreMinimal.h"

// Include della classe base per tutti i widget UI
#include "Blueprint/UserWidget.h"

// Include della classe base delle unità per riferimenti diretti
#include "PAASchifanoFrancesco/Units/UnitBase.h"

// Include di componenti UI utilizzati nel file .cpp
#include "Components/ProgressBar.h"
#include "Components/Border.h"

#include "StatusGameWidget.generated.h"

// Forward declaration di componenti UI usati nel Designer
class UButton;
class UTextBlock;
class UVerticalBox;
class UProgressBar;

/**
 * Classe che rappresenta il widget visivo per lo stato del gioco,
 * includendo la gestione della vita delle unità e il pulsante "End Turn".
 */
UCLASS()
class PAASCHIFANOFRANCESCO_API UStatusGameWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Override del metodo NativeConstruct chiamato all'inizializzazione del widget.
	 * In questo metodo si collegano gli eventi e si configura il pulsante di fine turno.
	 */
	virtual void NativeConstruct() override;

	/**
	 * Metodo chiamato quando si clicca sul pulsante di fine turno.
	 * Termina il turno attuale usando il TurnManager.
	 */
	UFUNCTION(BlueprintCallable)
	void OnClickedEndTurn();

	/**
	 * Attiva o disattiva il pulsante "End Turn" e ne cambia la visibilità.
	 * @param bIsVisible - true per mostrarlo e attivarlo, false per nasconderlo e disattivarlo.
	 */
	UFUNCTION()
	void ActiveButton(bool bIsVisible);

	/**
	 * Aggiunge graficamente la barra della vita per un'unità appena spawnata.
	 * @param Unit - puntatore all'unità da tracciare.
	 */
	void AddUnitStatus(AUnitBase* Unit);

	/**
	 * Aggiorna il valore della barra della vita di una specifica unità.
	 * @param Unit - puntatore all'unità.
	 * @param NewHealthPercent - percentuale di salute da applicare (tra 0.0 e 1.0).
	 */
	void UpdateUnitHealth(AUnitBase* Unit, float NewHealthPercent);

	/**
	 * Rimuove graficamente la barra della vita di un'unità quando muore.
	 * @param Unit - unità da eliminare dalla UI.
	 */
	void RemoveUnitStatus(AUnitBase* Unit);

protected:
	/** Riferimento al pulsante di fine turno, assegnato via Blueprint (BindWidget) */
	UPROPERTY(meta = (BindWidget))
	UButton* EndButton;

	/** Testo visualizzato sul pulsante (non modificato dinamicamente nel codice) */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ButtonText;

	/** Box verticale che contiene tutte le barre della vita delle unità */
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* UnitStatusBox;

private:
	/**
	 * Mappa che collega ogni unità a una coppia di widget:
	 * - la ProgressBar (barra della vita)
	 * - il contenitore grafico (UBorder) che la include
	 * Serve per aggiornare o rimuovere dinamicamente la UI delle unità.
	 */
	TMap<AUnitBase*, TPair<UProgressBar*, UBorder*>> UnitHealthBars;
};