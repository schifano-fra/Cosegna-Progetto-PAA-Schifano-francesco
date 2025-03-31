// Creato da: Schifano Francesco, 5469994

#pragma once

// Include principale del motore Unreal
#include "CoreMinimal.h"

// Include delle classi necessarie al funzionamento del widget
#include "PAASchifanoFrancesco/Core/MyGameMode.h"
#include "PAASchifanoFrancesco/Units/UnitBase.h"
#include "Blueprint/UserWidget.h"

#include "SelectPawn.generated.h"

// Forward declaration di componenti UI
class UButton;
class UTextBlock;

/**
 * Classe USelectPawn
 * Widget che permette al giocatore di selezionare una pedina (Brawler o Sniper) nella fase di piazzamento.
 * Espone i pulsanti e i testi associati ai due tipi di unità. Gestisce la selezione e la disattivazione dei pulsanti.
 */
UCLASS()
class PAASCHIFANOFRANCESCO_API USelectPawn : public UUserWidget
{
	GENERATED_BODY()

public:

	/**
	 * Metodo NativeConstruct
	 * Viene eseguito automaticamente all'inizializzazione del widget.
	 * Recupera i riferimenti al GameMode e PlacementManager e collega gli eventi dei pulsanti.
	 */
	virtual void NativeConstruct() override;

	/**
	 * Metodo NativeOnMouseButtonDown
	 * Permette di gestire l’interazione tramite tasto destro del mouse sopra i pulsanti.
	 * Se il tasto destro è premuto sopra uno dei due pulsanti, simula la selezione.
	 */
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/**
	 * Metodo DisableButtonForPawn
	 * Disattiva il pulsante corrispondente all’unità già selezionata, impedendo doppio piazzamento.
	 * @param PawnType - Tipo dell’unità selezionata (Sniper o Brawler)
	 */
	void DisableButtonForPawn(TSubclassOf<AUnitBase> PawnType);

	/**
	 * Metodo AreAllButtonsDisabled
	 * Ritorna true se entrambi i pulsanti sono disabilitati → il player ha terminato la selezione.
	 */
	bool AreAllButtonsDisabled() const;

protected:

	/** Puntatore al pulsante per selezionare il Brawler (collegato via UMG) */
	UPROPERTY(meta = (BindWidget))
	UButton* BrawlerButton;

	/** Puntatore al pulsante per selezionare lo Sniper (collegato via UMG) */
	UPROPERTY(meta = (BindWidget))
	UButton* SniperButton;

	/** Testo descrittivo per lo Sniper (collegato via UMG) */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* SniperText;

	/** Testo descrittivo per il Brawler (collegato via UMG) */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* BrawlerText;

private:

	/**
	 * Callback quando viene cliccato il pulsante Brawler.
	 * Imposta il tipo di pedina da piazzare su ABrawler.
	 */
	UFUNCTION()
	void OnBrawlerSelected();

	/**
	 * Callback quando viene cliccato il pulsante Sniper.
	 * Imposta il tipo di pedina da piazzare su ASniper.
	 */
	UFUNCTION()
	void OnSniperSelected();

	/** Riferimento al GameMode, recuperato a runtime */
	AMyGameMode* GM;

	/** Riferimento al PlacementManager, usato per settare la pedina scelta */
	APlacementManager* PlacementManager;
};